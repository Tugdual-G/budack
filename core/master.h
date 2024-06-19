#ifndef MASTER_H_
#define MASTER_H_
#include "budack_core.h"
#include <mpi.h>
#include <stdlib.h>

// sweet spot
#define ZOOM_CENTER_X -1
#define ZOOM_CENTER_Y 0.35

typedef struct {
  int world_size;
  unsigned int nx, ny, nx_redu, ny_redu, n_it;
  unsigned int redu_fact, j_ll_redu, i_ll_redu;
  MPI_Request *requ;
  pts_msg *recbuff;
  double *a, *b;
  uint16_t *Rmax, *Gmax, *Bmax;
  uint16_t *R, *G, *B;
} Fargs;

int master(int world_size, Param param, double a[2], double b[2]);

void recieve_and_render(uint16_t *R, uint16_t *G, uint16_t *B, double a[2],
                        double b[2], unsigned int nx, unsigned int ny,
                        int world_size, unsigned int cycles_per_update);

void recieve_and_draw(uint16_t *R, uint16_t *G, uint16_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size);

void draw_gray_into_RGB_buffer_8(uint8_t *RGB, uint16_t *gray1, uint16_t *gray2,
                                 uint16_t *gray3, size_t size_gray);

int callback(uint16_t *R, uint16_t *G, uint16_t *B, void *fargs);

void down_sample(uint16_t *in, uint16_t *out, unsigned int in_nx,
                 unsigned int in_ny, unsigned int redu_fact);

void define_zoom(double x_b[2], double y_b[2], unsigned int *i_ll,
                 unsigned int *j_ll, unsigned int nx_redu, unsigned int ny_redu,
                 unsigned int nx, unsigned int ny);

void zoom(uint16_t *in, uint16_t *out, unsigned int i_ll, unsigned int j_ll,
          unsigned int out_nx, unsigned int out_ny, unsigned int in_nx);

#endif // MASTER_H_
