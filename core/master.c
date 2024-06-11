#include "master.h"
#include "budack_core.h"
#include "tiff_images.h"
#include <math.h>
#include <mpi.h>
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
  recieve_and_draw(R_32, G_32, B_32, a, b, nx, ny, world_size);

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
