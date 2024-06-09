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

  ////////////////////////////////////////////////
  //   Most important parameters
  ////////////////////////////////////////////////

  unsigned int nx = 1 * e3; // Grid size x axis
  unsigned int maxit = 200; // maximum number of iteration per point
  unsigned int minit = 0;   // minimum iteration per point
  float D = 8; // Points per pixels of interest (i.e. number of points
               // independant of the domain size), higher = less noise
  float a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi
  unsigned int start = 500;
  unsigned int depth = maxit;

  struct Param param = {.nx = &nx,
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

    float max_memory;
    // in bytes
    max_memory = nx * ny * sizeof(unsigned int) * (3 + 3 * world_size) +
                 LENGTH_STRT * sizeof(double);
    // In GiB
    max_memory /= (float)1024 * 1024;
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

  srand((unsigned int)rank + (unsigned int)time(NULL));
  // There is no need to parallelise this part

  clock_t begin = clock();

  double *M_brdr = NULL;
  unsigned int length_brdr = LENGTH_STRT / world_size;
  M_brdr = (double *)calloc(2 * length_brdr, sizeof(double));
  if (M_brdr == NULL) {
    printf("Error no memory allocated to border points \n");
    exit(1);
  }
  unsigned char *M = NULL;
  M = (unsigned char *)calloc(nx * ny, sizeof(double));
  if (M == NULL) {
    printf("Error, no memory allocated for border points grid \n");
    exit(1);
  }

  // For short computation it is faster to not compute these points
  // and just use the hint files
  border(depth, length_brdr, M_brdr, M, start, a[0], b[0], dx, nx);
  // Saving a view of the points
  if (rank == 0) {
    save(HINTS_FNAME, M, sizeof(unsigned char), nx * ny);
  }
  free(M);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  unsigned int *histo0 = NULL, *histo1 = NULL, *histo2 = NULL,
               *histo_sum0 = NULL, *histo_sum1 = NULL, *histo_sum2 = NULL;

  if (rank == 0) {
    histo_sum0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (histo_sum0 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  } else if (rank == 1) {
    histo_sum1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (histo_sum1 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  } else if (rank == 2) {
    histo_sum2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (histo_sum2 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  }
  histo0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  histo1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  histo2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));

  if (histo0 == NULL || histo1 == NULL || histo2 == NULL) {
    printf("\n Error, no memory allocated for trajectories \n");
    exit(1);
  }

  D = D / world_size;

  MPI_Barrier(MPI_COMM_WORLD);

  clock_t end = clock();
  float t_comp;
  if (rank == 0) {
    begin = clock();
  }

  trajectories(nx, ny, a, b, histo0, histo1, histo2, D, maxit, minit, M_brdr,
               length_brdr);
  free(M_brdr);

  MPI_Reduce(histo0, histo_sum0, nx * ny, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  free(histo0);
  MPI_Reduce(histo1, histo_sum1, nx * ny, MPI_INT, MPI_SUM, 1, MPI_COMM_WORLD);
  free(histo1);
  MPI_Reduce(histo2, histo_sum2, nx * ny, MPI_INT, MPI_SUM, 2, MPI_COMM_WORLD);
  free(histo2);

  MPI_Barrier(MPI_COMM_WORLD);
  uint16_t *R_16 = NULL;
  uint16_t *G_16 = NULL;
  uint16_t *B_16 = NULL;
  if (rank == 0) {
    end = clock();
    t_comp = (float)(end - begin);
    t_comp = t_comp / CLOCKS_PER_SEC;
    printf("\nTime elapsed computing trajectories %f s \n", t_comp);
    // Storing variables on disk
    mirror_traj(ny, nx, histo_sum0); // Make the image symetric
    R_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
    G_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
    B_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
    if (!R_16 | !G_16 | !B_16) {
      printf("\n Error, no memory allocated for RGB arrays \n");
      exit(1);
    }
    normalize_uint_to_16bits(histo_sum0, R_16, nx * ny);
    free(histo_sum0);
    MPI_Recv(G_16, nx * ny, MPI_UINT16_T, 1, 10, MPI_COMM_WORLD, NULL);
    MPI_Recv(B_16, nx * ny, MPI_UINT16_T, 2, 20, MPI_COMM_WORLD, NULL);

  } else if (rank == 1) {
    mirror_traj(ny, nx, histo_sum1); // Make the image symetric
    G_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
    if (!G_16) {
      printf("\n Error, no memory allocated for G RGB arrays \n");
      exit(1);
    }
    normalize_uint_to_16bits(histo_sum1, G_16, nx * ny);
    free(histo_sum1);
    MPI_Send(G_16, nx * ny, MPI_UINT16_T, 0, 10, MPI_COMM_WORLD);

  } else if (rank == 2) {
    mirror_traj(ny, nx, histo_sum2); // Make the image symetric
    B_16 = (uint16_t *)calloc(nx * ny, sizeof(uint16_t));
    if (!B_16) {
      printf("\n Error, no memory allocated for B RGB arrays \n");
      exit(1);
    }
    normalize_uint_to_16bits(histo_sum2, B_16, nx * ny);
    free(histo_sum2);
    MPI_Send(B_16, nx * ny, MPI_UINT16_T, 0, 20, MPI_COMM_WORLD);
  }
  if (rank == 0) {
    unsigned outdir_str_len = strlen(param.output_dir);
    char filename[MAX_PATH_LENGTH + 21] = {'\0'};
    strncpy(filename, param.output_dir, MAX_PATH_LENGTH);
    if (param.output_dir[outdir_str_len - 1] != '/') {
      filename[outdir_str_len] = '/';
    }
    strncat(filename, "image.tiff", 11);
    write_tiff_16bitsRGB(filename, R_16, G_16, B_16, nx, ny);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize(); // finish MPI environment
  return 0;
}
