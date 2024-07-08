#include "budack_core.h"
#include "master.h"
#include "slave.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {

  int provided = 0;
  MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
  if (provided != MPI_THREAD_FUNNELED) {
    printf("Warning MPI did not provide MPI_THREAD_FUNNELED\n");
  }

  int world_size = -1; // number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank = -1; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (world_size < 3) {
    printf("Error: mpi should run at least 3 parallel processes. \n");
    exit(1);
  }

  ////////////////////////////////////////////////
  //   Most important parameters
  ////////////////////////////////////////////////

  unsigned int nx = 0;    // Grid size x axis
  unsigned int maxit = 0; // maximum number of iteration per point
  unsigned int minit = 0; // minimum iteration per point
  double density = 0; // Points per pixels of interest (i.e. number of points
                      // independant of the domain size), higher = less noise

  // boundaries of the complex domain
  double x_b[] = {DOMAIN_BOUND_XM, DOMAIN_BOUND_XP};
  double y_b[] = {-DOMAIN_BOUND_YP, DOMAIN_BOUND_YP};

  unsigned int depth = maxit;

  Param param = {
      .nx = &nx,
      .maxit = &maxit,
      .minit = &minit,
      .density = &density,
      .depth = &depth,
      .output_dir = "/tmp/",
      .cycles_per_update = 0,
  };

  parse(argc, argv, &param);

  ///////////////////////////////////////////////
  ///////////////////////////////////////////////

  // x and y are discretized at the midle of the cells
  double dx = (x_b[1] - x_b[0]) / nx;
  unsigned int ny = 2 * (unsigned int)(y_b[1] / dx);
  param.ny = &ny;
  y_b[0] = -((double)ny / 2) * dx;
  y_b[1] = ((double)ny / 2) * dx;

  switch (rank) {
  case 0: {
    master(world_size, param, x_b, y_b);
    break;
  }
  default: {
    slave(world_size, rank, param, x_b);
  }
  }

  MPI_Finalize(); // finish MPI environment
  return 0;
}
