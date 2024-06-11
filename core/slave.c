// #include "slave.h"
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

int slave(int world_size, int rank, Param param, double a[2]) {

  unsigned int nx = *param.nx, depth = *param.depth, maxit = *param.maxit,
               minit = *param.minit;
  double D = *param.D;

  double dx = (a[1] - a[0]) / nx;
  ////////////////////////////////////////////////
  //   Searching for points on the boundary
  ////////////////////////////////////////////////

  // There is no need to parallelise this part

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
  //   Cumputing the trajectories
  ////////////////////////////////////////////////

  trajectories(D / (double)world_size, maxit, minit, starting_pts, length_brdr,
               dx);
  free(starting_pts);

  return 0;
}
