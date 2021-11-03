#include "budack.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const int e6 = 1000000;
const int e3 = 1000;
#define dirname0 "output/"
#define dirname1 "output/traj0/"
char *paramfname = dirname1 "param.txt";
char *traj0fname = dirname1 "traj0.char";
char *traj1fname = dirname1 "traj1.char";
char *traj2fname = dirname1 "traj2.char";
char *traj0fname_uint = dirname1 "traj0.uint";
char *traj1fname_uint = dirname1 "traj1.uint";
char *traj2fname_uint = dirname1 "traj2.uint";
char *hintsfname = dirname1 "hints.char";

int main(int argc, char *argv[]) {
  /* chdir(".."); */
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
  float D = 16; // Points per pixels of interest (i.e. number of points
                // independant of the domain size), higher = less noise
  float a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi
  unsigned int start = 500;
  unsigned int depth = maxit;

  struct Param param = {
      .nx = &nx, .maxit = &maxit, .minit = &minit, .D = &D, .depth = &depth};

  parse(argc, argv, &param);

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (a[1] - a[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(b[1] / dx);
  b[0] = -((int)ny / 2) * dx;
  b[1] = ((int)ny / 2) * dx;

  if (rank == 0) {

    mkdir(dirname0, 0777);
    mkdir(dirname1, 0777);
    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny * nx);
    printf("maxit = %d ; minit = %d ; Points per pixels %.2f ; depth = %u \n",
           maxit, minit, D, depth);
    export_param(param, paramfname);
    printf("Depth of the data written to disk : %lu \n \n",
           sizeof(unsigned char));
    printf("Begin computation on %d cores \n", world_size);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

  clock_t begin = clock();

  long int Nborder = nx * ny / 40;

  double *M_brdr = NULL;
  M_brdr = (double *)calloc(2 * Nborder, sizeof(double));
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

  border(depth, Nborder, M_brdr, M, start, a[0], b[0], dx, nx);

  clock_t end = clock();
  float t_comp = (float)(end - begin);
  t_comp = t_comp / CLOCKS_PER_SEC;
  if (rank == 0) {
    printf("Core 0 found %ld border points in %f s\n\n", Nborder / 2, t_comp);
  }

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  srand((unsigned int)time(NULL));
  unsigned int *B0 = NULL, *B1 = NULL, *B2 = NULL, *B_sum0 = NULL,
               *B_sum1 = NULL, *B_sum2 = NULL;

  if (rank == 0) {
    B_sum0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (B_sum0 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  } else if (rank == 1) {
    B_sum1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (B_sum1 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  } else if (rank == 2) {
    B_sum2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
    if (B_sum2 == NULL) {
      printf("\n Error, no memory allocated for trajectories sum \n");
      exit(1);
    }
  }
  B0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));

  if (B0 == NULL || B1 == NULL || B2 == NULL) {
    printf("\n Error, no memory allocated for trajectories \n");
    exit(1);
  }
  D = D / world_size;

  if (rank == 0) {
    begin = clock();
  }

  trajectories(nx, ny, a, b, B0, B1, B2, D, maxit, minit, M_brdr, Nborder);

  MPI_Reduce(B0, B_sum0, nx * ny, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  MPI_Reduce(B1, B_sum1, nx * ny, MPI_INT, MPI_SUM, 1, MPI_COMM_WORLD);
  MPI_Reduce(B2, B_sum2, nx * ny, MPI_INT, MPI_SUM, 2, MPI_COMM_WORLD);
  // unsigned int arraysize[2] = {ny, nx};

  if (rank == 0) {
    end = clock();
    t_comp = (float)(end - begin);
    t_comp = t_comp / CLOCKS_PER_SEC;
    printf("\nTime elapsed computing trajectories %f s \n", t_comp);

    // Storing variables on disk
    // save("trajectories_data/arraysize.uint", arraysize, sizeof(arraysize));
    // save("trajectories_data/boundary.uint", M_brdr, sizeof(double) *
    // Nborder);
    mirror_traj(ny, nx, B_sum0); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum0, 1, traj0fname);
    save(traj0fname_uint, B_sum0, sizeof(unsigned int), nx * ny);
    save(hintsfname, M, sizeof(unsigned char), nx * ny);

  } else if (rank == 1) {
    mirror_traj(ny, nx, B_sum1); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum1, 1, traj1fname);
    save(traj1fname_uint, B_sum1, sizeof(unsigned int), nx * ny);

  } else if (rank == 2) {
    mirror_traj(ny, nx, B_sum2); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum2, 1, traj2fname);
    save(traj2fname_uint, B_sum2, sizeof(unsigned int), nx * ny);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank == 0) {
    free(B_sum0);
  }
  if (rank == 1) {
    free(B_sum1);
  }
  if (rank == 2) {
    free(B_sum2);
  }
  free(M_brdr);
  free(B0);
  free(B1);
  free(B2);
  if (rank == 0) {
    printf("allocated space freed \n");
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
