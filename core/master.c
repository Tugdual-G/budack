/*
** The master process gathers starting points to iterate,
** draw their trajectory in memory and plots the current state of the
** computation.
*/
#include "master.h"
#include "budack_core.h"
#include "opengl/render.h"
#include "tiff_images.h"
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KiB_SIZE 1024.0

// Returns a time difference in seconds
double get_time_diff_seconds(struct timespec t1, struct timespec t0);

int master(int world_size, Param param, double x_b[2], double y_b[2]) {
  /*
  ** Handles all of the master process actions.
  */

  printf("Number of cores : %i \n", world_size);

  // grid size
  unsigned int nx = *param.nx, ny = *param.ny;

  // escape time of the starting points
  unsigned int depth = *param.depth;

  // iteration range
  unsigned int maxit = *param.maxit, minit = *param.minit;

  // memory usage estimation in bytes
  double max_memory = 3.0 * nx * ny * (sizeof(uint16_t) + sizeof(uint16_t)) +
                      LENGTH_STRT * sizeof(double);

  // In GiB
  max_memory /= KiB_SIZE * KiB_SIZE;
  printf("Max memory usage :\x1b[32m %.0f MiB \x1b[0m\n", max_memory);

  printf("nx = %d ; ny = %d ; depth = %u \n", nx, ny, depth);
  printf("maxit = %d ; minit = %d ; Points per pixels %.2f \n", maxit, minit,
         *param.density);
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
  R_16 = (uint16_t *)calloc((size_t)nx * ny, sizeof(uint16_t));
  G_16 = (uint16_t *)calloc((size_t)nx * ny, sizeof(uint16_t));
  B_16 = (uint16_t *)calloc((size_t)nx * ny, sizeof(uint16_t));
  if (!B_16 || !R_16 || !G_16) {
    printf("\n Error, no memory allocated for trajectories sum \n");
    exit(1);
  }
  if (param.cycles_per_update != 0) {
    recieve_and_render(R_16, G_16, B_16, x_b, y_b, nx, ny, world_size,
                       param.cycles_per_update);
  } else {
    recieve_and_draw(R_16, G_16, B_16, x_b, y_b, nx, ny, world_size);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  clock_gettime(CLOCK_REALTIME, &time_1);
  printf("\nTime elapsed computing trajectories %lf s \n",
         get_time_diff_seconds(time_1, time_0) / 1e9);

  mirror_traj(ny, nx, R_16);
  mirror_traj(ny, nx, G_16);
  mirror_traj(ny, nx, B_16);
  normalize_16bits(R_16, (size_t)nx * ny);
  normalize_16bits(G_16, (size_t)nx * ny);
  normalize_16bits(B_16, (size_t)nx * ny);

  // Storing variables on disk
  unsigned outdir_str_len = strlen(param.output_dir);
  char filename[MAX_PATH_LENGTH + 21] = {'\0'};
  strncpy(filename, param.output_dir, MAX_PATH_LENGTH + 20 - outdir_str_len);
  strncat(filename, "image.tiff", 11);

  write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny);
  free(R_16);
  free(G_16);
  free(B_16);

  return 0;
}

void recieve_and_draw(uint16_t *R, uint16_t *G, uint16_t *B, double x_b[2],
                      double y_b[2], unsigned int nx, unsigned int ny,
                      int world_size) {
  /*
  ** Fills the RGB values.
  ** Input :
  **     - x_b, y_b     boundaries of the domain
  **     - nx, ny       size of the RGB arrays
  **     world_size     number of processes
  ** Output :
  **     - R, G, B      trajectories
  */
  clock_t t0 = 0, t = 0;
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
          draw_trajectories(R, recbuff[i].x, recbuff[i].y, recbuff[i].nit, x_b,
                            y_b, nx, ny);
          break;
        case 'g':
          draw_trajectories(G, recbuff[i].x, recbuff[i].y, recbuff[i].nit, x_b,
                            y_b, nx, ny);
          break;
        case 'b':
          draw_trajectories(B, recbuff[i].x, recbuff[i].y, recbuff[i].nit, x_b,
                            y_b, nx, ny);
          break;
        default:
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

void recieve_and_render(uint16_t *R, uint16_t *G, uint16_t *B, double x_b[2],
                        double y_b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update) {
  /*
  ** Recieves points computed by the slave processes and render them.
  ** input :
  **    - x_b, y_b           boundary of the domain
  **    - nx, ny             size of the R, G, B arrays
  **    - world_size         number of processes
  **    - cycles_per_update  number of recieves/store cycles to perform
  **                         between each rendering frame.
  **
  ** output :
  **    - R, G, B             arrays storing the points trajectories.
  */

  // Reduction factor in case the zoomed in display is used
  unsigned int redu_fact = 1;

  // subdomain of the trajectory to be plotted
  uint16_t *R_zoom = NULL, *G_zoom = NULL, *B_zoom = NULL;
  if (nx > MAX_RENDER_SIZE * 2 - 1) {
    redu_fact = nx / MAX_RENDER_SIZE;
    R_zoom =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    G_zoom =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    B_zoom =
        (uint16_t *)calloc(nx / redu_fact * ny / redu_fact, sizeof(uint16_t));
    if (!B_zoom | !R_zoom | !G_zoom) {
      printf("\n Error, no memory allocated for reduced RGB sum \n");
      exit(1);
    }
  } else {
    R_zoom = R;
    G_zoom = G;
    B_zoom = B;
  }

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
      .R = R_zoom,
      .G = G_zoom,
      .B = B_zoom,
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
      .x_b = x_b,
      .y_b = y_b,
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
    define_zoom(x_b, y_b, &args.i_ll_redu, &args.j_ll_redu, args.nx_redu,
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

void max16(const uint16_t *X, size_t n, uint16_t *max);

int callback(uint16_t *R_subdomain, uint16_t *G_subdomain,
             uint16_t *B_subdomain, void *fargs) {
  /*
  ** Callback function for the live plot.
  ** This function recieves the starting points computed by the slave process.
  ** Then the trajectories are drawn and, if needed a zoomed in version is used
  ** to fill the subdomain arrays, which will be used by the live render.
  **
  ** input :
  **     - R_, G_, B_subdomain  region of the trajectory arrays to be plotted
  **     - fargs                other arguments
  */

  // equals the number of complete slaves processes
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
          draw_trajectories(R, rec[i].x, rec[i].y, rec[i].nit, args->x_b,
                            args->y_b, args->nx, args->ny);
          break;
        case 'g':
          draw_trajectories(G, rec[i].x, rec[i].y, rec[i].nit, args->x_b,
                            args->y_b, args->nx, args->ny);
          break;
        case 'b':
          draw_trajectories(B, rec[i].x, rec[i].y, rec[i].nit, args->x_b,
                            args->y_b, args->nx, args->ny);
          break;
        default:
          break;
        }
      }

    } else {
      ++completion_flag;
    }
  }

  if (args->redu_fact > 1) {
    /* downsample(R, R_subdomain, args->nx, args->ny, args->redu_fact); */
    /* downsample(G, G_subdomain, args->nx, args->ny, args->redu_fact); */
    /* downsample(B, B_subdomain, args->nx, args->ny, args->redu_fact); */
    zoom(R, R_subdomain, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
    zoom(G, G_subdomain, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
    zoom(B, B_subdomain, args->i_ll_redu, args->j_ll_redu, args->nx_redu,
         args->ny_redu, args->nx);
  }
  max16(R_subdomain, (size_t)args->nx_redu * args->ny_redu, args->Rmax);
  max16(G_subdomain, (size_t)args->nx_redu * args->ny_redu, args->Gmax);
  max16(B_subdomain, (size_t)args->nx_redu * args->ny_redu, args->Bmax);
  return (completion_flag != (args->world_size - 1));
}

void downsample(const uint16_t *in, uint16_t *out, unsigned int in_nx,
                unsigned int in_ny, unsigned int redu_fact) {
  /*
  ** Fills the output array with a downsampled version of the input array.
  */

  unsigned int out_nx = in_nx / redu_fact;
  unsigned int out_ny = in_ny / redu_fact;
  unsigned int stencil_size = redu_fact * redu_fact;

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
      out[i_out * out_nx + j_out] /= stencil_size;
    }
  }
}

void zoom(const uint16_t *in, uint16_t *out, unsigned int i_ll,
          unsigned int j_ll, unsigned int out_nx, unsigned int out_ny,
          unsigned int in_nx) {
  /*
  ** Extract a subregion defined by a bounding box in the input array
  ** Input :
  **     - in              input array
  **     - in_nx           size of the input array
  **     - i_ll, j_ll      bounding box low left corner
  **     - out_nx, out_ny  size of the output array
  **
  ** Output :
  **     - out         zoomed in region
  */

  for (unsigned int i = 0; i < out_ny; ++i) {
    for (unsigned int j = 0; j < out_nx; ++j) {
      out[i * out_nx + j] = in[(i + i_ll) * in_nx + j + j_ll];
    }
  }
}

void define_zoom(const double x_b[2], const double y_b[2], unsigned int *i_ll,
                 unsigned int *j_ll, unsigned int nx_zoom, unsigned int ny_zoom,
                 unsigned int nx, unsigned int ny) {
  /*
  ** Returns a bounding box aroud the zoom center defined by the define macros
  ** ZOOM_CENTER_X (resp _Y).
  **
  ** Input :
  **     - x_b, y_b          boundaries of the domain
  **     - nx_zoom, ny_zoom  size of the zoom region
  **     - nx, ny            size of the whole domain
  **
  ** Return :
  **     - i_ll, j_ll        bounding box low left corner postion
  **
  */

  double dx = (x_b[1] - x_b[0]) / nx;
  double x0 = ZOOM_CENTER_X;
  double y0 = ZOOM_CENTER_Y;
  double x_ll = (x0 - dx * nx_zoom / 2.0); // low left corner
  double y_ll = (y0 - dx * ny_zoom / 2.0);
  double x_tr = (x0 + dx * nx_zoom / 2.0); // top right corner
  double y_tr = (y0 + dx * ny_zoom / 2.0);

  // Ensure that the bounding box is included in the domain.
  if (y_ll > y_b[0]) {
    *i_ll = (unsigned int)((y_ll - y_b[0]) / dx);
  } else {
    *i_ll = 0;
  }
  if (y_tr >= y_b[1] - dx) {
    *i_ll = ny - ny_zoom;
  }

  if (x_ll > x_b[0]) {
    *j_ll = (unsigned int)((x_ll - x_b[0]) / dx);
  } else {
    *j_ll = 0;
  }
  if (x_tr >= x_b[1]) {
    *j_ll = nx - nx_zoom;
  }
}

double get_time_diff_seconds(struct timespec t1, struct timespec t0) {
  // Return time difference in seconds.
  return (1e9 * (double)(t1.tv_sec - t0.tv_sec) +
          (double)(t1.tv_nsec - t0.tv_nsec));
}

void max16(const uint16_t *X, size_t n, uint16_t *max) {
  *max = 0;
  for (size_t i = 0; i < n; ++i) {
    if (X[i] > *max) {
      *max = X[i];
    }
  }
}
