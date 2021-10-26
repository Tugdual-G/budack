#include "budack.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int e6 = 1000000;
const int e3 = 1000;

struct Param {
  long int N;
  unsigned int nx, ny;
  unsigned int maxit;
  unsigned int minit;
  float a[2];
  float b[2];
  unsigned int *B;
  unsigned int *B_sum;
  unsigned int *M;
  double *M_brdr;
};

int main() {
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
  unsigned int minit = 50;  // minimum iteration per point
  long int D = 500 * e3;    // density of point, higher = less noise
  float a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (a[1] - a[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(b[1] / dx);
  b[0] = -((int)ny / 2) * dx;
  b[1] = ((int)ny / 2) * dx;

  if (rank == 0) {
    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny * nx);
    printf("maxit = %d ; minit = %d ; density of points %.1e\n", maxit, minit,
           (double)D);
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

  double *M_brdr;
  long int Nborder = nx * ny / 20;
  unsigned int depth = maxit;

  M_brdr = (double *)calloc(Nborder, sizeof(double));
  if (M_brdr == NULL) {
    printf("Error, no memory space allocated for computing");
    return 0;
  }

  border(depth, Nborder, M_brdr);

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
  unsigned int *B, *B_sum;
  B = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  B_sum = (unsigned int *)calloc(nx * ny, sizeof(unsigned int));
  D = D / world_size;
  if (B_sum == NULL) {
    printf("Error, no memory space (heap) allocated for storing results");
    return 0;
  }

  if (rank == 0) {
    begin = clock();
  }

  trajectories(nx, ny, a, b, B, D, maxit, minit, M_brdr, Nborder);

  MPI_Reduce(B, B_sum, nx * ny, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  unsigned int arraysize[2] = {ny, nx};

  if (rank == 0) {
    end = clock();
    t_comp = (float)(end - begin);
    t_comp = t_comp / CLOCKS_PER_SEC;
    printf("\nTime elapsed computing trajectories %f s \n", t_comp);

    // Storing variables on disk
    save("trajectories_data/arraysize.uint", arraysize, sizeof(arraysize));
    save("trajectories_data/boundary.uint", M_brdr, sizeof(double) * Nborder);
    save_chargrayscale(ny, nx, B_sum, "trajectories_data/b.char");
  }

  MPI_Barrier(MPI_COMM_WORLD);
  free(M_brdr);
  free(B);
  free(B_sum);
  if (rank == 0) {
    printf("allocated space freed \n");
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
