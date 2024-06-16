/* budack_core.h */
#ifndef BUDACK_CORE_H_
#define BUDACK_CORE_H_

#include <stdint.h>

#define PTS_MSG_SIZE 128
#define LENGTH_STRT 50000
#define MAX_PATH_LENGTH 490
#define PARAM_FNAME "param.txt"
#define HINTS_FNAME "hintsfiles/hints"

// A define an area of reference to compute the density
// of points.
#define AREA 9.0

typedef struct {
  double x;
  double y;
  unsigned int nit;
  unsigned int color;
} Pts_msg;

typedef struct {
  unsigned int *nx, *ny, *maxit, *minit, *depth;
  double *D;
  char *output_dir;
  unsigned int cycles_per_update;
} Param;

void draw_trajectories(uint32_t *M, double x0, double y0, unsigned int nit,
                       double *x_b, double *y_b, unsigned int nx,
                       unsigned int ny);

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

#endif // BUDACK_CORE_H_
