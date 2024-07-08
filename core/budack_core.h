/*
** This file declare all the lower level core functions.
** These functions are used by the master and the slave processes.
*/
#ifndef BUDACK_CORE_H_
#define BUDACK_CORE_H_

#include <stdint.h>
#include <stdlib.h>

#define DOMAIN_BOUND_XM -2.3
#define DOMAIN_BOUND_XP 1.3
#define DOMAIN_BOUND_YP 1.5

#define PTS_MSG_SIZE 20     // number of points sent in each message
#define LENGTH_STRT 50000   // number of hints/poles to generate
#define MAX_PATH_LENGTH 512 // maximum length of the output path string
#define PARAM_FNAME "param.txt"
#define HINTS_FNAME "hintsfiles/hints"

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
  double *density;
  char *output_dir;
  unsigned int cycles_per_update;
} Param;

void draw_trajectories(uint16_t *M, double x0, double y0, unsigned int nit,
                       const double *x_b, const double *y_b, unsigned int nx,
                       unsigned int ny);
/*
** Fills the input array with the trajectory of the provided starting point.
** Input :
**  - x0, y0    starting points
**  - nit       number of iterations to perform
**  - x_b, y_b  boundaries of the domain
**  - nx, ny    size of the output array M
**
** Output :
**  - M         output array
*/

void trajectories(double density, unsigned int maxit, unsigned int minit,
                  const double *starting_pts, unsigned int length_strt,
                  double dx);
/* Finds the starting points which are then sent to the master process.
** Tree ranges of escape times are used to generate rgb values.
** Input :
**   - D              density
**   - maxit, minit   maximum and minimum number of iterations (escape time)
**   - starting_pts   points which serves as center of the probability
*                     distribution from which the new points are generated.
*    - length_strt    number of points in starting_pts
*    - dx             grid step
*/

int border(unsigned int depth, long int length_strt, double *starting_pts);
/*
** Fills starting_pts with a list of random points at the boundary
** of the mandelbrot set, if a file already exist, load them, else,
** generate the points with the border_start function and write them
** to file. The binary file has the structure [y0, x0, y1, x1, y2, ...]
** These points are used later as poles when randomly generating starting
** points for the trajectories.
**
**  input :
**     - depth         number of iterations of the starting points
**                     (escape time)
**
**     - length_strt   number of starting points
**
** output :
**     - starting_pts  starting points
*/

void border_start(unsigned int depth, double *starting_pts,
                  unsigned int length_start);
/*
** Generate uniformlly distributed points on the complex plane, and returns
** the ones having escape time close to the depth parameter.
** Input :
**     - depth           desired number of iteration (escape time)
**     - length_start    number of starting points
**
** Output :
**     - starting_pts    starting points having the right escape times
*/

void save(const char fname[], void *data, size_t size, size_t n_elements);

void mirror_traj(unsigned int ny, unsigned int nx, uint16_t *B);

void parse(int argc, char *argv[], Param *param);

void export_param(Param param);

void write_progress(double density);

void save_rgb_uint8(uint16_t *R, uint16_t *G, uint16_t *B, char *filename,
                    unsigned nx, unsigned ny);

#endif // BUDACK_CORE_H_
