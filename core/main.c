#include "budack_core.h"
#include "master.h"
#include "slave.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const int e6 = 1000000;
const int e3 = 1000;

int main(int argc, char *argv[]) {

  int provided;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
  if (provided != MPI_THREAD_FUNNELED) {
    printf("Warning MPI did not provide MPI_THREAD_FUNNELED\n");
  }

  int world_size; // number of processes
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
  unsigned int minit = 20;  // minimum iteration per point
  double D = 8; // Points per pixels of interest (i.e. number of points
                // independant of the domain size), higher = less noise
  double a[2] = {-2.3, 1.3}, b[2] = {-1.5, 1.5}; // size of the domain a+bi
  unsigned int depth = maxit;

  Param param = {
      .nx = &nx,
      .maxit = &maxit,
      .minit = &minit,
      .D = &D,
      .depth = &depth,
      .output_dir = "/tmp/",
      .cycles_per_update = 0,
  };

  parse(argc, argv, &param);

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (a[1] - a[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(b[1] / dx);
  param.ny = &ny;
  b[0] = -((int)ny / 2) * dx;
  b[1] = ((int)ny / 2) * dx;

  switch (rank) {
  case 0: {
    master(world_size, param, a, b);
    break;
  }
  default: {
    slave(world_size, rank, param, a);
  }
  }

  MPI_Finalize(); // finish MPI environment
  return 0;
}
