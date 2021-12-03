/* budack_core.h */

void trajectories(unsigned int nx, unsigned int ny, float x_b[2], float y_b[2],
                  unsigned int *M_traj0, unsigned int *M_traj1,
                  unsigned int *M_traj2, float D, int maxit, int minit,
                  double *starting_pts, unsigned int length_strt);

void border(unsigned int depth, long int length_strt, double *starting_pts,
            unsigned char *M, unsigned int start, float a0, float b0, double dx,
            unsigned int nx);

void border_start(unsigned int depth, double *starting_pts, unsigned char *M,
                  unsigned int start, float a0, float b0, double dx,
                  unsigned int nx);

void save(const char fname[], void *data, unsigned int size,
          unsigned int n_elements);

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B);

void save_char_grayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                         unsigned char weight, const char fname[]);

void save_uint_grayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                         float gamma, const char fname[]);

struct Param {
  unsigned int *nx, *ny, *maxit, *minit, *depth;
  float *D;
};

void parse(int argc, char *argv[], struct Param *param);

void export_param(struct Param param, const char filename[]);

void cd_to_root_dir(char *arg0);
