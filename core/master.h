/*
** The master process gathers starting points to iterate and
** plots the current state of the computation.
*/
#ifndef MASTER_H_
#define MASTER_H_
#include "budack_core.h"
#include <mpi.h>
#include <stdlib.h>

// sweet spot
#define ZOOM_CENTER_X -1.0
#define ZOOM_CENTER_Y 0.35

typedef struct {
  int world_size;
  unsigned int nx, ny, nx_redu, ny_redu, n_it;
  unsigned int redu_fact, j_ll_redu, i_ll_redu;
  MPI_Request *requ;
  pts_msg *recbuff;
  double *x_b, *y_b;
  uint16_t *Rmax, *Gmax, *Bmax;
  uint16_t *R, *G, *B;
} Fargs;

int master(int world_size, Param param, double x_b[2], double y_b[2]);
/*
** Handles all of the master process actions.
*/

void recieve_and_render(uint16_t *R, uint16_t *G, uint16_t *B, double x_b[2],
                        double y_b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update);
/*
** Recieves points computed by the slave processes and render them.
** input :
**    - x_b, y_b           boundary of the domain
**    - nx, ny             size of the R, G, B arrays
**    - world_size         number of processes
**    - cycles_per_update  number of recieves/store cycles to perform
**                         between each rendering frame.
**
** output :
**    - R, G, B             arrays storing the points trajectories.
*/

void recieve_and_draw(uint16_t *R, uint16_t *G, uint16_t *B, double x_b[2],
                      double y_b[2], unsigned int nx, unsigned int ny,
                      int world_size);
/*
** Fills the RGB values.
** Input :
**     - x_b, y_b     boundaries of the domain
**     - nx, ny       size of the RGB arrays
**     world_size     number of processes
** Output :
**     - R, G, B      trajectories
*/

int callback(uint16_t *R, uint16_t *G, uint16_t *B, void *fargs);
/*
** Callback function for the live plot.
** This function recieves the starting points computed by the slave process.
** Then the trajectories are drawn and, if needed a zoomed in version is used
** to fill the subdomain arrays, which will be used by the live render.
**
** input :
**     - R_, G_, B_subdomain  region of the trajectory arrays to be plotted
**     - fargs                other arguments
*/

void down_sample(const uint16_t *in, uint16_t *out, unsigned int in_nx,
                 unsigned int in_ny, unsigned int redu_fact);
/*
** Fills the output array with a downsampled version of the input array.
*/

void define_zoom(const double x_b[2], const double y_b[2], unsigned int *i_ll,
                 unsigned int *j_ll, unsigned int nx_zoom, unsigned int ny_zoom,
                 unsigned int nx, unsigned int ny);
/*
** Returns a bounding box aroud the zoom center defined by the define macros
** ZOOM_CENTER_X (resp _Y).
**
** Input :
**     - x_b, y_b          boundaries of the domain
**     - nx_zoom, ny_zoom  size of the zoom region
**     - nx, ny            size of the whole domain
**
** Return :
**     - i_ll, j_ll        bounding box low left corner postion
**
*/

void zoom(const uint16_t *in, uint16_t *out, unsigned int i_ll,
          unsigned int j_ll, unsigned int out_nx, unsigned int out_ny,
          unsigned int in_nx);
/*
** Extract a subregion defined by a bounding box in the input array
** Input :
**     - in              input array
**     - in_nx           size of the input array
**     - i_ll, j_ll      bounding box low left corner
**     - out_nx, out_ny  size of the output array
**
** Output :
**     - out         zoomed in region
*/

#endif // MASTER_H_
