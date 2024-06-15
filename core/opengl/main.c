#include "render.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int readfile_uint32(char *filename, size_t size, uint32_t *buffer) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    printf("Error opening file \n");
    return 0;
  }
  if (fread(buffer, sizeof(uint32_t), size, fp) < size) {
    printf("Error reading file \n");
    fclose(fp);
    return 0;
  }
  fclose(fp);
  return 1;
}

typedef struct {
  unsigned int nit;
  double x;
  double y;
  char color;
} pts_msg;

typedef struct {
  int world_size;
  unsigned int nx, ny, n_it, redu_fact, nx_redu, ny_redu;
  MPI_Request *requ;
  pts_msg *recbuff;
  double *a, *b;
  uint32_t *Rmax, *Gmax, *Bmax;
  uint32_t *R, *G, *B;
} Fargs;

int callback(uint32_t *R, uint32_t *G, uint32_t *B, void *fargs) {
  Fargs *args = (Fargs *)fargs;
  args->Rmax = 0;
  args->Gmax = 0;
  args->Bmax = 0;
  for (size_t i = 0; i < (size_t)args->nx * args->ny; ++i) {
    if (R[i] > *args->Rmax) {
      *args->Rmax = R[i];
    }
    if (G[i] > *args->Gmax) {
      *args->Gmax = G[i];
    }
    if (B[i] > *args->Bmax) {
      *args->Bmax = B[i];
    }
  }
  return 1;
}

int main() {
  // load and generate the texture
  int width = 1000, heigth = 832;
  size_t size = width * heigth;
  int fileflag = 0;

  uint32_t *R = (uint32_t *)malloc(size * sizeof(uint32_t));
  fileflag += readfile_uint32("r.uint32", size, R);
  uint32_t *G = (uint32_t *)malloc(size * sizeof(uint32_t));
  fileflag += readfile_uint32("g.uint32", size, G);
  uint32_t *B = (uint32_t *)malloc(size * sizeof(uint32_t));
  fileflag += readfile_uint32("b.uint32", size, B);
  if (fileflag != 3) {
    printf("Error loading file \n");
  }

  Render_object rdr_obj = {
      .width = width,
      .height = heigth,
      .Runit = 0,
      .Gunit = 1,
      .Bunit = 2,
      .Rmax = MAX_UINT,
      .Gmax = MAX_UINT,
      .Bmax = MAX_UINT,
      .R = R,
      .G = G,
      .B = B,
  };

  Fargs args = {
      .world_size = 1,
      .nx = width,
      .ny = heigth,
      .nx_redu = width,
      .ny_redu = heigth,
      .n_it = 1,
      .Rmax = &rdr_obj.Rmax,
      .Gmax = &rdr_obj.Gmax,
      .Bmax = &rdr_obj.Bmax,
      .R = R,
      .G = G,
      .B = B,
  };

  render_init(&rdr_obj);
  printf(" ok1 \n");
  render_loop(&rdr_obj, callback, &args);
  printf(" ok2 \n");
  render_finalize(&rdr_obj);
  return 0;
}
