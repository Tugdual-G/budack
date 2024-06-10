/* budack_core.h */
#ifndef BUDACK_CORE_H_
#define BUDACK_CORE_H_

#include <stdint.h>

#define MAX_PATH_LENGTH 490
#define PARAM_FNAME "param.txt"
#define HINTS_FNAME "hints.tiff"

#define PTS_MSG_SIZE 10

// A define an area of reference to compute the density
// of points.
#define AREA 9.0

typedef struct {
  unsigned int nit;
  double x;
  double y;
  char color;
} pts_msg;

void draw_trajectories(uint32_t *M, double x0, double y0, unsigned int nit,
                       double *x_b, double *y_b, unsigned int nx,
                       unsigned int ny);
void trajectories(double D, int maxit, int minit, double *starting_pts,
                  unsigned int length_strt, double dx);

int border(unsigned int depth, long int length_strt, double *starting_pts);

void border_start(unsigned int depth, double *starting_pts,
                  unsigned int length_start);

void save(const char fname[], void *data, unsigned int size,
          unsigned int n_elements);

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B);

struct Param {
  unsigned int *nx, *ny, *maxit, *minit, *depth;
  double *D;
  char *output_dir;
};

void parse(int argc, char *argv[], struct Param *param);

void export_param(struct Param param);

void write_progress(double density);

#endif // ASDF_H_
