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

int master(int world_size, Param param, double a[2], double b[2]) {

  printf("Number of cores : %i \n", world_size);

  double max_memory;

  unsigned int nx = *param.nx, ny = *param.ny, depth = *param.depth,
               maxit = *param.maxit, minit = *param.minit;
  double D = *param.D;
  // in bytes
  max_memory = 3 * nx * ny * (sizeof(uint16_t) + sizeof(uint32_t)) +
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
  clock_t t_begin = clock();
  border(depth, length_brdr, NULL);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  uint32_t *R_32 = NULL, *G_32 = NULL, *B_32 = NULL;
  R_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  G_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  B_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
  if (!B_32 | !R_32 | !G_32) {
    printf("\n Error, no memory allocated for trajectories sum \n");
    exit(1);
  }
  if (param.cycles_per_update != 0) {
    recieve_and_render(R_32, G_32, B_32, a, b, nx, ny, world_size,
                       param.cycles_per_update);
  } else {
    recieve_and_draw(R_32, G_32, B_32, a, b, nx, ny, world_size);
  }

  printf("\nTime elapsed computing trajectories %f s \n",
         (double)(clock() - t_begin) / CLOCKS_PER_SEC);

  uint16_t *R_16 = NULL;
  uint16_t *G_16 = NULL;
  uint16_t *B_16 = NULL;

  // Storing variables on disk
  mirror_traj(ny, nx, R_32); // Make the image symetric
  mirror_traj(ny, nx, G_32); // Make the image symetric
  mirror_traj(ny, nx, B_32); // Make the image symetric
  R_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  G_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  B_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
  if (!R_16 | !G_16 | !B_16) {
    printf("\n Error, no memory allocated for RGB arrays \n");
    exit(1);
  }
  normalize_32_to_16bits(R_32, R_16, nx * ny);
  free(R_32);
  normalize_32_to_16bits(G_32, G_16, nx * ny);
  free(G_32);
  normalize_32_to_16bits(B_32, B_16, nx * ny);
  free(B_32);

  unsigned outdir_str_len = strlen(param.output_dir);
  char filename[MAX_PATH_LENGTH + 21] = {'\0'};
  strncpy(filename, param.output_dir, MAX_PATH_LENGTH);
  if (param.output_dir[outdir_str_len - 1] != '/') {
    filename[outdir_str_len] = '/';
  }
  strncat(filename, "image.tiff", 11);
  /* save_rgb_uint8(R_16, G_16, B_16, "/tmp/rgb.uint8", nx, ny); */
  write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny);
  free(R_16);
  free(G_16);
  free(B_16);

  return 0;
}

void recieve_and_draw(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
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

void recieve_and_render(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                        double b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update) {
  uint8_t *RGB = (uint8_t *)calloc(nx * ny * 3, sizeof(uint8_t));
  if (!RGB) {
    printf("Error: RGB not allocated in recieve_and_render \n");
    exit(1);
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

  Fargs args = {
      .world_size = world_size,
      .nx = nx,
      .ny = ny,
      .n_it = cycles_per_update,
      .requ = &requ,
      .recbuff = recbuff,
      .R = R,
      .G = G,
      .B = B,
      .a = a,
      .b = b,
  };

  Render_object rdr_obj = render_init(RGB, nx, ny, 3);
  render_loop(rdr_obj, RGB, nx, ny, callback, &args);
  render_finalize(&rdr_obj);
  free(recbuff);
  free(RGB);
  write_progress(-2);
  /* printf("\nmaster waiting time : %lf s \n", (double)t / CLOCKS_PER_SEC); */
}

int callback(uint8_t *data, void *fargs) {
  Fargs *args = (Fargs *)fargs;
  unsigned int it = 0;
  static int completion_flag = 0;
  pts_msg *rec = args->recbuff;

  while ((completion_flag < (args->world_size - 1)) && (it < args->n_it)) {
    ++it;
    MPI_Start(args->requ);
    MPI_Wait(args->requ, MPI_STATUS_IGNORE);
    /* MPI_Recv(recbuff, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, */
    /*          MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
     */
    if (rec[0].color) {
      for (unsigned int i = 0; i < PTS_MSG_SIZE; ++i) {
        switch (rec[i].color) {
        case 'r':
          draw_trajectories(args->R, rec[i].x, rec[i].y, rec[i].nit, args->a,
                            args->b, args->nx, args->ny);
          break;
        case 'g':
          draw_trajectories(args->G, rec[i].x, rec[i].y, rec[i].nit, args->a,
                            args->b, args->nx, args->ny);
          break;
        case 'b':
          draw_trajectories(args->B, rec[i].x, rec[i].y, rec[i].nit, args->a,
                            args->b, args->nx, args->ny);
          break;
        }
      }

    } else {
      ++completion_flag;
    }
  }
  draw_gray_into_RGB_buffer_8(data, args->R, args->G, args->B,
                              (args->nx) * (args->ny));

  return (completion_flag != (args->world_size - 1));
}

double sigmoidal_contrast(double alpha, double beta, double x);
void draw_gray_into_RGB_buffer_8(uint8_t *RGB, uint32_t *gray1, uint32_t *gray2,
                                 uint32_t *gray3, size_t size_gray) {

  uint32_t max1 = 0, max2 = 0, max3 = 0;

  {
    for (size_t k = 0; k < size_gray; ++k) {
      if (gray1[k] > max1) {
        max1 = gray1[k];
      }
    }

    for (size_t k = 0; k < size_gray; ++k) {
      if (gray2[k] > max2) {
        max2 = gray2[k];
      }
    }

    for (size_t k = 0; k < size_gray; ++k) {
      if (gray3[k] > max3) {
        max3 = gray3[k];
      }
    }
  }

  for (size_t k = 0; k < size_gray; ++k) {
    RGB[3 * k] = 255.0 * sigmoidal_contrast(0.05, 10, gray1[k] / (double)max1);
    RGB[3 * k + 1] =
        255.0 * sigmoidal_contrast(0.1, 10, gray2[k] / (double)max2);
    RGB[3 * k + 2] =
        255.0 * sigmoidal_contrast(0.1, 10, gray3[k] / (double)max3);
  }
}

double sigmoidal_contrast(double alpha, double beta, double x) {
  // x must be normalized to 1
  return (1 / (1 + exp(beta * (alpha - x))) - 1 / (1 + exp(beta * (alpha)))) /
         (1 / (1 + exp(beta * (alpha - 1))) - 1 / (1 + exp(beta * alpha)));
}

void mirror(unsigned int ny, unsigned int nx, uint8_t *X) {
  // Ensure symetry and add density by adding a mirrored version
  // of the image to itself.
  unsigned long k;
  unsigned int i, j;
  uint8_t b;

  for (i = 0; i < ny; i++) {
    for (j = 0; j < nx; j++) {
      k = (unsigned long)ny * nx - (1 + i) * nx + j;
      b = X[i * nx + j];
      X[i * nx + j] += X[k];
      X[k] += b;
    }
  }
}
