#include "master.h"
#include "budack_core.h"
#include "opengl/render.h"
#include "tiff_images.h"
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

double get_time_diff_nano(struct timespec t1, struct timespec t0);
int master(int world_size, Param param, double a[2], double b[2]) {

  printf("Number of cores : %i \n", world_size);

  double max_memory;

  unsigned int nx = *param.nx, ny = *param.ny, depth = *param.depth,
               maxit = *param.maxit, minit = *param.minit;
  double D = *param.D;
  // in bytes
  max_memory = 3 * nx * ny * (sizeof(uint16_t) + sizeof(uint16_t)) +
               LENGTH_STRT * sizeof(double);
  // In GiB
  max_memory /= (double)1024 * 1024;
  printf("Max memory usage :\x1b[32m %.0f MiB \x1b[0m\n", max_memory);

  printf("nx = %d ; ny = %d ; depth = %u \n", nx, ny, depth);
  printf("maxit = %d ; minit = %d ; Points per pixels %.2f \n", maxit, minit,
         D);
  export_param(param);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

  unsigned int length_brdr = LENGTH_STRT / (world_size - 1);
  struct timespec time_1, time_0;
  clock_gettime(CLOCK_REALTIME, &time_0);
  border(depth, length_brdr, NULL);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  uint16_t *R_16 = NULL, *G_16 = NULL, *B_16 = NULL;
  R_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  G_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  B_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  if (!B_16 | !R_16 | !G_16) {
    printf("\n Error, no memory allocated for trajectories sum \n");
    exit(1);
  }
  if (param.cycles_per_update != 0) {
    recieve_and_render(R_16, G_16, B_16, a, b, nx, ny, world_size,
                       param.cycles_per_update);
  } else {
    recieve_and_draw(R_16, G_16, B_16, a, b, nx, ny, world_size);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  clock_gettime(CLOCK_REALTIME, &time_1);
  printf("\nTime elapsed computing trajectories %lf s \n",
         get_time_diff_nano(time_1, time_0) / 1E9);

  mirror_traj(ny, nx, R_16);
  mirror_traj(ny, nx, G_16);
  mirror_traj(ny, nx, B_16);
  normalize_16bits(R_16, nx * ny);
  normalize_16bits(G_16, nx * ny);
  normalize_16bits(B_16, nx * ny);

  // Storing variables on disk
  unsigned outdir_str_len = strlen(param.output_dir);
  char filename[MAX_PATH_LENGTH + 21] = {'\0'};
  strncpy(filename, param.output_dir, MAX_PATH_LENGTH);
  if (param.output_dir[outdir_str_len - 1] != '/') {
    filename[outdir_str_len] = '/';
  }
  strncat(filename, "image.tiff", 11);

  write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny);
  free(R_16);
  free(G_16);
  free(B_16);

  return 0;
}

void recieve_and_draw(uint16_t *R, uint16_t *G, uint16_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size) {
  clock_t t0, t = 0;
  write_progress(0);
  MPI_Request requ;
  int completion_flag = 0;

  pts_msg *recbuff = (pts_msg *)malloc(sizeof(pts_msg) * PTS_MSG_SIZE);
  if (!recbuff) {
    printf("Error: recbuff not allocated \n");
    exit(1);
  }
  MPI_Recv_init(recbuff, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE,
                MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &requ);

  while (completion_flag < (world_size - 1)) {
    t0 = clock();
    MPI_Start(&requ);
    MPI_Wait(&requ, MPI_STATUS_IGNORE);
    /* MPI_Recv(recbuff, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, */
    /*          MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); */
    t += clock() - t0;

    if (recbuff[0].color) {
      for (unsigned int i = 0; i < PTS_MSG_SIZE; ++i) {
        switch (recbuff[i].color) {
        case 'r':
          draw_trajectories(R, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        case 'g':
          draw_trajectories(G, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        case 'b':
          draw_trajectories(B, recbuff[i].x, recbuff[i].y, recbuff[i].nit, a, b,
                            nx, ny);
          break;
        }
      }

    } else {
      ++completion_flag;
    }
  }
  free(recbuff);
  write_progress(-2);
  printf("\nmaster waiting time : %lf s \n", (double)t / CLOCKS_PER_SEC);
}

void recieve_and_render(uint16_t *R, uint16_t *G, uint16_t *B, double a[2],
                        double b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update) {

  unsigned int redu_fact = 1;
  uint16_t *R_reduced = NULL, *G_reduced = NULL, *B_reduced = NULL;
  if (nx > MAX_RENDER_SIZE * 2 - 1) {
    redu_fact = nx / MAX_RENDER_SIZE;
    R_reduced =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    G_reduced =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    B_reduced =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    if (!B_reduced | !R_reduced | !G_reduced) {
      printf("\n Error, no memory allocated for reduced RGB sum \n");
      exit(1);
    }
  } else {
    R_reduced = R;
    G_reduced = G;
    B_reduced = B;
  }

  /* clock_t t0, t = 0; */
  write_progress(0);
  MPI_Request requ;
  pts_msg *recbuff = (pts_msg *)malloc(sizeof(pts_msg) * PTS_MSG_SIZE);
  if (!recbuff) {
    printf("Error: recbuff not allocated \n");
    exit(1);
  }
  MPI_Recv_init(recbuff, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE,
                MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &requ);

  Render_object rdr_obj = {
      .width = nx / redu_fact,
      .max_width = nx,
      .height = ny / redu_fact,
      .max_height = ny,
      .Runit = 0,
      .Gunit = 1,
      .Bunit = 2,
      .Rmax = MAX_UINT16,
      .Gmax = MAX_UINT16,
      .Bmax = MAX_UINT16,
      .R = R_reduced,
      .G = G_reduced,
      .B = B_reduced,
  };

  Fargs args = {
      .world_size = world_size,
      .nx = nx,
      .ny = ny,
      .nx_redu = nx / redu_fact,
      .ny_redu = ny / redu_fact,
      .n_it = cycles_per_update,
      .requ = &requ,
      .recbuff = recbuff,
      .a = a,
      .b = b,
      .Rmax = &rdr_obj.Rmax,
      .Gmax = &rdr_obj.Gmax,
      .Bmax = &rdr_obj.Bmax,
      .R = R,
      .G = G,
      .B = B,
      .redu_fact = redu_fact,
  };
  if (redu_fact > 1) {
    // define the lower left corner of the zoomed rendered region;
    define_zoom(a, b, &args.i_ll_redu, &args.j_ll_redu, args.nx_redu,
                args.ny_redu, nx, ny);
  }

  rdr_obj.i_ll = &args.i_ll_redu;
  rdr_obj.j_ll = &args.j_ll_redu;

  render_init(&rdr_obj);
  render_loop(&rdr_obj, callback, &args);
  render_finalize(&rdr_obj);
  free(recbuff);
  write_progress(-2);
  printf("\nR_max=%u, G_max=%u, B_max=%u \n", *args.Rmax, *args.Gmax,
         *args.Bmax);
}

void downsample(uint16_t *in, uint16_t *out, unsigned int in_nx,
                unsigned int in_ny, unsigned int redu_fact);
void max16(uint16_t *X, size_t n, uint16_t *max);
int callback(uint16_t *R_reduced, uint16_t *G_reduced, uint16_t *B_reduced,
             void *fargs) {

  static int completion_flag = 0;

  Fargs *args = (Fargs *)fargs;
  uint16_t *R = args->R, *G = args->G, *B = args->B;
  pts_msg *rec = args->recbuff;

  unsigned int it = 0;
  while ((completion_flag < (args->world_size - 1)) && (it < args->n_it)) {
    ++it;
    MPI_Start(args->requ);
    MPI_Wait(args->requ, MPI_STATUS_IGNORE);
    if (rec[0].color) {
      for (unsigned int i = 0; i < PTS_MSG_SIZE; ++i) {
        switch (rec[i].color) {
        case 'r':
          draw_trajectories(R, rec[i].x, rec[i].y, rec[i].nit, args->a, args->b,
                            args->nx, args->ny);
          break;
        case 'g':
          draw_trajectories(G, rec[i].x, rec[i].y, rec[i].nit, args->a, args->b,
                            args->nx, args->ny);
          break;
        case 'b':
          draw_trajectories(B, rec[i].x, rec[i].y, rec[i].nit, args->a, args->b,
                            args->nx, args->ny);
          break;
        }
      }

    } else {
      ++completion_flag;
    }
  }

  if (args->redu_fact > 1) {
    /* downsample(R, R_reduced, args->nx, args->ny, args->redu_fact); */
    /* downsample(G, G_reduced, args->nx, args->ny, args->redu_fact); */
    /* downsample(B, B_reduced, args->nx, args->ny, args->redu_fact); */
    zoom(R, R_reduced, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
    zoom(G, G_reduced, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
    zoom(B, B_reduced, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
  }
  max16(R_reduced, args->nx_redu * args->ny_redu, args->Rmax);
  max16(G_reduced, args->nx_redu * args->ny_redu, args->Gmax);
  max16(B_reduced, args->nx_redu * args->ny_redu, args->Bmax);
  return (completion_flag != (args->world_size - 1));
}

void max16(uint16_t *X, size_t n, uint16_t *max) {
  *max = 0;
  for (size_t i = 0; i < n; ++i) {
    if (X[i] > *max) {
      *max = X[i];
    }
  }
}

void downsample(uint16_t *in, uint16_t *out, unsigned int in_nx,
                unsigned int in_ny, unsigned int redu_fact) {

  unsigned int out_nx = in_nx / redu_fact, out_ny = in_ny / redu_fact,
               area = redu_fact * redu_fact;
  for (unsigned int i_out = 0; i_out < out_ny; ++i_out) {
    for (unsigned int j_out = 0; j_out < out_nx; ++j_out) {
      out[i_out * out_nx + j_out] = 0;
    }
    for (unsigned int i_in = i_out * redu_fact; i_in < (i_out + 1) * redu_fact;
         ++i_in) {
      for (unsigned int j_out = 0; j_out < out_nx; ++j_out) {
        for (unsigned int j_in = j_out * redu_fact;
             j_in < (j_out + 1) * redu_fact; ++j_in) {
          out[i_out * out_nx + j_out] += in[i_in * in_nx + j_in];
        }
      }
    }
    for (unsigned int j_out = 0; j_out < out_nx; ++j_out) {
      out[i_out * out_nx + j_out] /= area;
    }
  }
}

void zoom(uint16_t *in, uint16_t *out, unsigned int i_ll, unsigned int j_ll,
          unsigned int out_nx, unsigned int out_ny, unsigned int in_nx) {

  for (unsigned int i = 0; i < out_ny; ++i) {
    for (unsigned int j = 0; j < out_nx; ++j) {
      out[i * out_nx + j] = in[(i + i_ll) * in_nx + j + j_ll];
    }
  }
}

void define_zoom(double x_b[2], double y_b[2], unsigned int *i_ll,
                 unsigned int *j_ll, unsigned int nx_redu, unsigned int ny_redu,
                 unsigned int nx, unsigned int ny) {

  double dx = (x_b[1] - x_b[0]) / nx, x0 = ZOOM_CENTER_X, y0 = ZOOM_CENTER_Y;
  double x_ll = (x0 - dx * nx_redu / 2.0);
  double y_ll = (y0 - dx * ny_redu / 2.0);
  double x_tr = (x0 + dx * nx_redu / 2.0);
  double y_tr = (y0 + dx * ny_redu / 2.0);

  if (y_ll > y_b[0]) {
    *i_ll = (y_ll - y_b[0]) / dx;
  } else {
    *i_ll = 0;
  }
  if (y_tr >= y_b[1] - dx) {
    *i_ll = ny - ny_redu;
  }

  if (x_ll > x_b[0]) {
    *j_ll = (x_ll - x_b[0]) / dx;
  } else {
    *j_ll = 0;
  }
  if (x_tr >= x_b[1]) {
    *j_ll = nx - nx_redu;
  }
}

double get_time_diff_nano(struct timespec t1, struct timespec t0) {
  return (1000000000 * (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec));
}
