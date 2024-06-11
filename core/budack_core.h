/* budack_core.h */
#ifndef BUDACK_CORE_H_
#define BUDACK_CORE_H_

#include <stdint.h>

#define MAX_PATH_LENGTH 490
#define PARAM_FNAME "param.txt"
#define HINTS_FNAME "hints.tiff"

#define PTS_MSG_SIZE 20

// A define an area of reference to compute the density
// of points.
#define AREA 9.0

typedef struct {
  unsigned int nit;
  double x;
  double y;
  char color;
} pts_msg;

typedef struct {
  unsigned int *nx, *ny, *maxit, *minit, *depth;
  double *D;
  char *output_dir;
} Param;

void draw_trajectories(uint32_t *M, double x0, double y0, unsigned int nit,
                       double *x_b, double *y_b, unsigned int nx,
                       unsigned int ny);

void recieve_and_draw(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size);

void trajectories(double D, unsigned int maxit, unsigned int minit,
                  double *starting_pts, unsigned int length_strt, double dx);

int border(unsigned int depth, long int length_strt, double *starting_pts);

void border_start(unsigned int depth, double *starting_pts,
                  unsigned int length_start);

void save(const char fname[], void *data, unsigned int size,
          unsigned int n_elements);

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B);

void parse(int argc, char *argv[], Param *param);

void export_param(Param param);

void write_progress(double density);

void save_rgb_uint8(uint16_t *R, uint16_t *G, uint16_t *B, char *filename,
                    unsigned nx, unsigned ny);
#endif // ASDF_H_
