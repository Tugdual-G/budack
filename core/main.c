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

const int e6 = 1000000;
const int e3 = 1000;

const unsigned int LENGTH_STRT = 50000;

int main(int argc, char *argv[]) {

  MPI_Init(NULL, NULL); // initialize MPI environment
  int world_size;       // number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (world_size < 3) {
    printf("Error: mpi should run at least 3 parallel processes. \n");
    exit(1);
  }

  ////////////////////////////////////////////////
  //   Most important parameters
  ////////////////////////////////////////////////

  unsigned int nx = 1 * e3; // Grid size x axis
  unsigned int maxit = 200; // maximum number of iteration per point
  unsigned int minit = 0;   // minimum iteration per point
  double D = 8; // Points per pixels of interest (i.e. number of points
                // independant of the domain size), higher = less noise
  double a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi
  unsigned int depth = maxit;

  Param param = {.nx = &nx,
                 .maxit = &maxit,
                 .minit = &minit,
                 .D = &D,
                 .depth = &depth,
                 .output_dir = "/tmp/"};

  parse(argc, argv, &param);

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (a[1] - a[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(b[1] / dx);
  param.ny = &ny;
  b[0] = -((int)ny / 2) * dx;
  b[1] = ((int)ny / 2) * dx;

  if (rank == 0) {
    printf("Number of cores : %i \n", world_size);
    // FIXME The creation of the directories should be done
    // during the installation.

    double max_memory;
    // in bytes
    max_memory = nx * ny * sizeof(unsigned int) * (3 + 3 * world_size) +
                 LENGTH_STRT * sizeof(double);
    // In GiB
    max_memory /= (double)1024 * 1024;
    printf("Max memory usage :\x1b[32m %.0f MiB \x1b[0m\n", max_memory);

    printf("nx = %d ; ny = %d ; depth = %u \n", nx, ny, depth);
    printf("maxit = %d ; minit = %d ; Points per pixels %.2f \n", maxit, minit,
           D);
    export_param(param);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

  clock_t begin = clock();

  double *starting_pts = NULL;
  unsigned int length_brdr = LENGTH_STRT / (world_size - 1);

  if (rank != 0) {
    srand((unsigned int)rank + (unsigned int)time(NULL));
    starting_pts = (double *)calloc(2 * length_brdr, sizeof(double));
    if (starting_pts == NULL) {
      printf("Error no memory allocated to border points \n");
      exit(1);
    }
  }

  // For short computation it is faster to not compute these points
  // and just use the hint files
  border(depth, length_brdr, starting_pts);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  uint32_t *R_32 = NULL, *G_32 = NULL, *B_32 = NULL;

  if (rank == 0) {
    R_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
    G_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
    B_32 = (uint32_t *)calloc(nx * ny, sizeof(uint32_t));
    if (!B_32 | !R_32 | !G_32) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  clock_t end = clock();
  double t_comp;

  switch (rank) {
  case 0: {
    write_progress(0);
    begin = clock();

    double current_density = 0;
    int completion_flag = 0;
    pts_msg *recbuff = (pts_msg *)malloc(sizeof(pts_msg) * PTS_MSG_SIZE);
    if (!recbuff) {
      printf("Error: recbuff not allocated \n");
      exit(1);
    }
    MPI_Status status;

    while (completion_flag < (world_size - 1)) {

      MPI_Recv(recbuff, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE,
               MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (recbuff[0].color) {
        for (unsigned int i = 0; i < PTS_MSG_SIZE; ++i) {
          switch (recbuff[i].color) {
          case 'r':
            draw_trajectories(R_32, recbuff[i].x, recbuff[i].y, recbuff[i].nit,
                              a, b, nx, ny);
            break;
          case 'g':
            draw_trajectories(G_32, recbuff[i].x, recbuff[i].y, recbuff[i].nit,
                              a, b, nx, ny);
            break;
          case 'b':
            draw_trajectories(B_32, recbuff[i].x, recbuff[i].y, recbuff[i].nit,
                              a, b, nx, ny);
            /* current_density += recbuff[i].nit * dx * dx / (AREA * D); */
            break;
          }
          /* write_progress(current_density); */
        }

      } else {
        ++completion_flag;
      }
    }
    free(recbuff);
    write_progress(-2);
    break;
  }
  default: {
    trajectories(D / (double)world_size, maxit, minit, starting_pts,
                 length_brdr, dx);
    free(starting_pts);
  }
  }

  switch (rank) {
  case 0: {
    uint16_t *R_16 = NULL;
    uint16_t *G_16 = NULL;
    uint16_t *B_16 = NULL;

    end = clock();
    t_comp = (double)(end - begin);
    t_comp = t_comp / CLOCKS_PER_SEC;
    printf("\nTime elapsed computing trajectories %f s \n", t_comp);
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
    write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny);
    free(R_16);
    free(G_16);
    free(B_16);
  }
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
