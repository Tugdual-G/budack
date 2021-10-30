#include "budack.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int e6 = 1000000;
const int e3 = 1000;

struct Param {
  unsigned int *nx, *maxit, *minit,
      *start; // This is the number of starting points from where to begin
  long int *D;
};

void parse(int argc, char *argv[], struct Param *param) {
  if (argc > 1) {
    *(*param).nx = atoi(argv[1]);
    *(*param).maxit = atoi(argv[2]);
    *(*param).minit = atoi(argv[3]);
    *(*param).start = atoi(argv[4]);
    *(*param).D = atoi(argv[5]);
  }
}

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
  unsigned int maxit = 800; // maximum number of iteration per point
  unsigned int minit = 0;   // minimum iteration per point
  long int D = 4; // Points per pixels of interest (i.e. number of points
                  // independant of the domain size), higher = less noise
  float a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi
  unsigned int start = 500;

  struct Param param = {
      .nx = &nx, .maxit = &maxit, .minit = &minit, .start = &start, .D = &D};

  parse(argc, argv, &param);

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (a[1] - a[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(b[1] / dx);
  b[0] = -((int)ny / 2) * dx;
  b[1] = ((int)ny / 2) * dx;

  if (rank == 0) {
    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny * nx);
    printf("maxit = %d ; minit = %d ; Points per pixels %ld\n", maxit, minit,
           D);
    printf("Depth of the data written to disk : %lu \n \n",
           sizeof(unsigned int));
    printf("Begin computation on %d cores \n", world_size);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

  clock_t begin = clock();

  long int Nborder = nx * ny / 20;
  unsigned int depth = maxit;

  double *M_brdr = (double *)calloc(Nborder, sizeof(double));
  unsigned char *M = (unsigned char *)calloc(nx * ny, sizeof(double));
  if (M_brdr == NULL) {
    printf("Error, no memory space allocated for computing");
    return 0;
  }

  border(depth, Nborder, M_brdr, M, start, a[0], b[0], dx, nx);

  clock_t end = clock();
  float t_comp = (float)(end - begin);
  t_comp = t_comp / CLOCKS_PER_SEC;
  if (rank == 0) {
    printf("Core 0 found %ld border points in %f s\n\n", Nborder, t_comp);
  }

  ////////////////////////////////////////////////
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  srand((unsigned int)time(NULL));
  unsigned int *B0, *B1, *B2, *B_sum0, *B_sum1, *B_sum2;
  B0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B_sum0 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B_sum1 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B_sum2 = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  D = D / world_size;
  if (B_sum2 == NULL) {
    printf("Error, no memory space (heap) allocated for storing results");
    return 0;
  }

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
    save_chargrayscale(ny, nx, B_sum0, 3, "trajectories_data/traj0.char");
    fill(M, M_brdr, Nborder * 2, nx, ny, dx, a[0], b[0]);
    save("trajectories_data/hints.char", M, nx * ny);

  } else if (rank == 1) {
    mirror_traj(ny, nx, B_sum1); // Make the image symetric
    save_chargrayscale(ny, nx, B_sum1, 2, "trajectories_data/traj1.char");

  } else if (rank == 2) {
    mirror_traj(ny, nx, B_sum2); // Make the image symetric
    save_chargrayscale(ny, nx, B_sum2, 1, "trajectories_data/traj2.char");
  }
  MPI_Barrier(MPI_COMM_WORLD);
  free(M_brdr);
  free(B0);
  free(B_sum0);
  free(B1);
  free(B_sum1);
  free(B2);
  free(B_sum2);
  if (rank == 0) {
    printf("allocated space freed \n");
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
