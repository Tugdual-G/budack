/* budack_core.h */
#ifndef BUDACK_CORE_H_
#define BUDACK_CORE_H_

#define MAX_PATH_LENGTH 490
#define PARAM_FNAME "param.txt"
#define HINTS_FNAME "hints.tiff"

#include <stdint.h>

void trajectories(unsigned int nx, unsigned int ny, double x_b[2],
                  double y_b[2], unsigned int *M_traj0, unsigned int *M_traj1,
                  unsigned int *M_traj2, double D, int maxit, int minit,
                  double *starting_pts, unsigned int length_strt);

void border(unsigned int depth, long int length_strt, double *starting_pts,
            uint8_t *M, unsigned int start, double a0, double b0, double dx,
            unsigned int nx);

void border_start(unsigned int depth, double *starting_pts, uint8_t *M,
                  unsigned int start, double a0, double b0, double dx,
                  unsigned int nx);

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
