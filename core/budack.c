#include "budack_core.h"
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
const char *paramfname = dirname1 "param.txt";
const char *traj0fname = dirname1 "traj0.char";
const char *traj1fname = dirname1 "traj1.char";
const char *traj2fname = dirname1 "traj2.char";
const char *traj0fname_uint = dirname1 "traj0.uint";
const char *traj1fname_uint = dirname1 "traj1.uint";
const char *traj2fname_uint = dirname1 "traj2.uint";
const char *hintsfname = dirname1 "hints.char";

const unsigned int LENGTH_STRT = 50000;

int main(int argc, char *argv[]) {

  // This change the working directory to .../budack/
  // This is ugly, but seems to work as long as the
  // interior architecture and location of the executables
  // is preserved. (need core/ and output/ )
  cd_to_root_dir(argv[0]);

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

  struct Param param = {
      .nx = &nx, .maxit = &maxit, .minit = &minit, .D = &D, .depth = &depth};

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
    // FIXME The creation of the directories should be done
    // during the installation.
    mkdir(dirname0, 0777);
    mkdir(dirname1, 0777);

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
    export_param(param, paramfname);
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
    save(hintsfname, M, sizeof(unsigned char), nx * ny);
  }
  free(M);

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

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

  MPI_Barrier(MPI_COMM_WORLD);

  clock_t end = clock();
  float t_comp;
  if (rank == 0) {
    begin = clock();
  }

  trajectories(nx, ny, a, b, B0, B1, B2, D, maxit, minit, M_brdr, length_brdr);
  free(M_brdr);

  MPI_Reduce(B0, B_sum0, nx * ny, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  free(B0);
  MPI_Reduce(B1, B_sum1, nx * ny, MPI_INT, MPI_SUM, 1, MPI_COMM_WORLD);
  free(B1);
  MPI_Reduce(B2, B_sum2, nx * ny, MPI_INT, MPI_SUM, 2, MPI_COMM_WORLD);
  free(B2);

  if (rank == 0) {
    end = clock();
    t_comp = (float)(end - begin);
    t_comp = t_comp / CLOCKS_PER_SEC;
    printf("Time elapsed computing trajectories %f s \n", t_comp);

    // Storing variables on disk
    mirror_traj(ny, nx, B_sum0); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum0, 1, traj0fname);
    save(traj0fname_uint, B_sum0, sizeof(unsigned int), nx * ny);

  } else if (rank == 1) {
    mirror_traj(ny, nx, B_sum1); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum1, 1, traj1fname);
    save(traj1fname_uint, B_sum1, sizeof(unsigned int), nx * ny);

  } else if (rank == 2) {
    mirror_traj(ny, nx, B_sum2); // Make the image symetric
    save_char_grayscale(ny, nx, B_sum2, 1, traj2fname);
    save(traj2fname_uint, B_sum2, sizeof(unsigned int), nx * ny);
  }
  free(B_sum0);
  free(B_sum1);
  free(B_sum2);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize(); // finish MPI environment
  return 0;
}
