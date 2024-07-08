/*
** This file define the slaves processes actions.
** Slaves processes finds starting points with good trajectory
** property (long, divergent ...).
** Then the points are sent to the master process.
*/
#include "slave.h"
#include "budack_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int slave(int world_size, int rank, Param param, const double x_b[2]) {
  /*
  ** Handle the slave processes.
  ** Input :
  **    - world_size   number of mpi processes
  **    - rank         process rank
  **    - param        parameters
  **    - x_b          real part boundaries of the complex domain
  */

  // grid size
  unsigned int nx = *param.nx;
  // escape time of the poles/hints
  unsigned int depth = *param.depth;
  // iteration range
  unsigned int maxit = *param.maxit, minit = *param.minit;
  // grid step
  double dx = (x_b[1] - x_b[0]) / nx;

  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  double *starting_pts = NULL;
  unsigned int length_brdr = LENGTH_STRT / (world_size - 1);

  srand((unsigned int)rank + (unsigned int)time(NULL));
  starting_pts = (double *)calloc(2 * length_brdr, sizeof(double));
  if (starting_pts == NULL) {
    printf("Error no memory allocated to border points \n");
    exit(1);
  }

  border(depth, length_brdr, starting_pts);

  ////////////////////////////////////////////////
  //   Computing the trajectories
  ////////////////////////////////////////////////

  trajectories(*param.density / (double)world_size, maxit, minit, starting_pts,
               length_brdr, dx);
  free(starting_pts);

  return 0;
}
