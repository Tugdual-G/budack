#ifndef MASTER_H_
#define MASTER_H_
#include "budack_core.h"
#include "opengl/render.h"
#include <mpi.h>
#include <stdlib.h>

typedef struct {
  int world_size;
  unsigned int nx, ny, n_it, redu_fact, nx_redu, ny_redu;
  MPI_Request *requ;
  Pts_msg *recbuff;
  double *a, *b;
  uint32_t Rmax, Gmax, Bmax;
  uint32_t *R, *G, *B;
  clock_t waiting_t;
} Fargs;

int master(int world_size, Param param, double a[2], double b[2]);

void recieve_and_render(double a[2], double b[2], unsigned int nx,
                        unsigned int ny, int world_size,
                        unsigned int cycles_per_update);

void recieve_and_draw(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size);

void draw_gray_into_RGB_buffer_8(uint8_t *RGB, uint32_t *gray1, uint32_t *gray2,
                                 uint32_t *gray3, size_t size_gray);

int callback(Render_object *rdr_obj, void *fargs);
#endif // MASTER_H_
