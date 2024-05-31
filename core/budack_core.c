#include "budack_core.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PI 3.141592654

// A define an area of reference to compute the density
// of points.
const long int A = 9;

double gaussrand(double dx) {
  // generate a random double.
  // Gaussian distribution.
  static double U, V;
  static int phase = 0;
  double Z;

  if (phase == 0) {
    U = (rand() + 1.) / (RAND_MAX + 2.);
    V = rand() / (RAND_MAX + 1.);
    Z = sqrt(-2 * log(U)) * sin(2 * PI * V);
  } else
    Z = sqrt(-2 * log(U)) * cos(2 * PI * V);

  phase = 1 - phase;

  return Z * dx;
}

double randomfloat(double min, double max) {
  // Uniform distribution.
  double r, range = max - min;
  r = (double)rand() / (double)(RAND_MAX);
  r = min + r * range;
  return r;
}

void trajectories(unsigned int nx, unsigned int ny, float x_b[2], float y_b[2],
                  unsigned int *restrict M_traj0,
                  unsigned int *restrict M_traj1,
                  unsigned int *restrict M_traj2, float D, int maxit, int minit,
                  double *restrict starting_pts, unsigned int length_strt) {
  // This where the trajectories are computed. Tree ranges of escape times are
  // used for rgb.
  // M_traj0 store the lowest escape time range of trajectories, and M_traj2 the
  // highest.

  // For parallel processing
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Locating the points in space
  // y for imaginary points
  // dx is the step of the grid
  double dx, x, y, x0, y0, x2, y2;

  // - itraj : number of trajectories satisfying the
  //           higgher minimal escape time.
  // - it : number of iteration for one trajectory.
  long unsigned int it = 0, itraj = 0;

  // - npts : total numbers of points visited for
  //          the higgest escape time range.
  long long unsigned int npts = 0;

  // ij : indexes of the visited points in the array.
  unsigned int ij[maxit * 2 + 1], i;
  char diverge = 0;

  // Current density.
  float current_D = 0;

  // Definition of the 3 ranges of escape time,
  int maxit0 = minit + (maxit - minit) / 3.0,
      maxit1 = minit + 2 * (maxit - minit) / 3.0;

  // To obtain an uniform density of points for each ranges, we fix some ratios.
  unsigned char n0, n1;
  unsigned char n0_max = maxit / maxit0, n1_max = maxit / maxit1;

  dx = (x_b[1] - x_b[0]) / nx;
  while (current_D < D) {
    // generate starting points
    y0 = starting_pts[(2 * itraj) % length_strt] + gaussrand(0.01);
    x0 = starting_pts[(2 * itraj + 1) % length_strt] + gaussrand(0.01);

    it = 0;
    x = x0;
    y = y0;
    x2 = x * x;
    y2 = y * y;
    diverge = 0;

    ij[it * 2] = (y - y_b[0]) / dx;
    ij[it * 2 + 1] = (x - x_b[0]) / dx;

    while (it < maxit && diverge == 0) {
      // Storing the trajectories
      n0 = 0;
      n1 = 0;
      y = 2 * x * y + y0;
      x = x2 - y2 + x0;
      x2 = x * x;
      y2 = y * y;
      it++;
      ij[it * 2] = (y - y_b[0]) / dx;
      ij[it * 2 + 1] = (x - x_b[0]) / dx;
      if (x2 + y2 > 4) {
        diverge = 1;
      }
    }
    if (diverge == 1 && it > minit) {
      if (it < maxit0 && n0 < n0_max) {
        n0++;
        for (i = 0; i < it; i += 2) {
          if (ij[i] >= 0 && ij[i] < ny && ij[i + 1] >= 0 && ij[i + 1] < nx) {
            *(M_traj0 + nx * ij[i] + ij[i + 1]) += 1;
          }
        }
      } else if (it < maxit1 && n1 < n1_max) {
        n1++;
        for (i = 0; i < it; i += 2) {
          if (ij[i] >= 0 && ij[i] < ny && ij[i + 1] >= 0 && ij[i + 1] < nx) {
            *(M_traj1 + nx * ij[i] + ij[i + 1]) += 1;
          }
        }
      } else {
        for (i = 0; i < it; i += 2) {
          if (ij[i] >= 0 && ij[i] < ny && ij[i + 1] >= 0 && ij[i + 1] < nx) {
            *(M_traj2 + nx * ij[i] + ij[i + 1]) += 1;
          }
        }
        itraj++;
        npts += it;
        current_D = (npts * dx * dx) / A;
      }
      if (rank == 0 && itraj % 10 == 0) {
        printf("\rPoints per pixel per core %-.4f/%.4f ", current_D, D);
        fflush(stdout);
      }
    }
  }
  if (rank == 0) {
    printf("\x1B[2K \r");
  }
}

void border(unsigned int depth, long int length_strt,
            double *restrict starting_pts, unsigned char *restrict M,
            unsigned int start, float a0, float b0, double dx,
            unsigned int nx) {
  // Fill starting_pts with a list of random points at the boundary
  // of the mandelbrot set, if a file already exist, load them, else,
  // generate the points with the border_start function and write them
  // to file. The binary file has the structure [y0, x0, y1, x1, y2, ...]
  // These points are used later as poles when randomly generating starting
  // points for the trajectories.

  // For parallel execution
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  int world_size; // number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Creating the name of the file we want to read or write to.
  // Each file depth can only be a multiple of 20 to avoid
  // exessive acumulation of hints files.
  char filename[100];
  // depth can't be bigger than 10^7.
  char depth_str[10];
  if (depth > 10000000 || depth < 11) {
    printf("\e[1;31mERROR: \e[0;37mdepth cannot be greater than 10000000 or "
           "smaller than 11\n");
    MPI_Finalize();
    exit(1);
  }
  // The depth is rounded to the nearest multiple of 20
  depth = ((depth - 11) / 20 + 1) * 20;
  strcpy(filename, "core/hints");
  snprintf(depth_str, 9, "%u", depth);
  strncat(filename, depth_str, 99);
  strncat(filename, ".double", 99);

  double *total_pts = NULL;
  FILE *fp = NULL;
  fp = fopen(filename, "rb");
  if (fp == NULL) {
    // If we can't find the file we create it.
    if (rank == 0) {
      printf("%s not found, creating file : \n", filename);
      total_pts =
          (double *)malloc(length_strt * 2 * world_size * sizeof(double));
      if (total_pts == NULL) {
        printf("\e[1;31mERROR: \e[0;37mno memory allocated to save starting "
               "pts \n");
        exit(1);
      }
    }

    border_start(depth, starting_pts, M, length_strt, a0, b0, dx, nx);
    MPI_Gather(starting_pts, length_strt * 2, MPI_DOUBLE, total_pts,
               length_strt * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      save(filename, total_pts, sizeof(double), world_size * 2 * length_strt);
      free(total_pts);
    }
  } else {
    // If we find the file we load the points.
    fseek(fp, length_strt * sizeof(double) * 2 * rank, SEEK_SET);
    fread(starting_pts, sizeof(double), 2 * length_strt, fp);
    fclose(fp);
  }
}

void border_start(unsigned int depth, double *starting_pts, unsigned char *M,
                  unsigned int start, float a0, float b0, double dx,
                  unsigned int nx) {
  // Found random points close to the border of the Mdlbrt set
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  double x, y, x0, y0, x2, y2;
  unsigned int it = 0;
  unsigned int k = 0;
  unsigned char mindepth = depth * 0.8;
  unsigned int i, j;

  while (k < start) {
    it = 0;

    x0 = randomfloat(-2, 0.5);
    y0 = randomfloat(0, 1.5);

    x = x0;
    y = y0;
    x2 = x * x;
    y2 = y * y;

    while (it < depth && x2 + y2 < 4) {
      y = 2 * x * y + y0;
      x = x2 - y2 + x0;
      x2 = x * x;
      y2 = y * y;
      it++;
    }
    if (it < depth && it >= mindepth) {
      *(starting_pts + k * 2) = y0;
      *(starting_pts + k * 2 + 1) = x0;
      i = (y0 - b0) / dx;
      j = (x0 - a0) / dx;
      *(M + i * nx + j) = 255;
      k++;
      if (rank == 0) {
        printf("\rGenerating file : %u / %u", k, start);
        fflush(stdout);
      }
    }
  }
  if (rank == 0) {
    printf("\n");
  }
}

void save(const char fname[], void *data, unsigned int size,
          unsigned int n_elements) {
  FILE *fp = NULL;
  fp = fopen(fname, "wb");
  if (fp == NULL) {
    printf("\e[1;31mERROR: \e[0;37mopening file %s, cannot save.\n", fname);
    exit(1);
  }
  fwrite(data, size, n_elements, fp);
  fclose(fp);
}

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B) {
  // Ensure symetry and add density by adding a mirrored version
  // of the image to itself.
  unsigned long k;
  unsigned int i, j;
  unsigned int b;

  for (i = 0; i < ny; i++) {
    for (j = 0; j < nx; j++) {
      k = (unsigned long)ny * nx - (1 + i) * nx + j;
      b = B[i * nx + j];
      B[i * nx + j] += B[k];
      B[k] += b;
    }
  }
}

void save_char_grayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                         unsigned char weight, const char fname[]) {
  unsigned long size = nx * ny;
  unsigned char *B_c = NULL;
  B_c = (unsigned char *)malloc(sizeof(unsigned char) * size);
  if (B_c == NULL) {
    printf(
        "\e[1;31mERROR: \e[0;37mno memory allocated to save char grayscale \n");
    exit(1);
  }

  unsigned long k;
  unsigned int bmax = 0;

  for (k = 0; k < size; k++) {
    if (*(B + k) > bmax) {
      bmax = *(B + k);
    }
  }
  for (k = 0; k < size; k++) {
    *(B_c + k) = (double)*(B + k) * 255 / (weight * bmax);
  }

  FILE *fptr;
  fptr = fopen(fname, "wb");
  if (fptr == NULL) {
	  printf("ERROR in save_char_grayscale, could not create file %s\n",fname);
  }
  fwrite(B_c, sizeof(unsigned char), size, fptr);
  fclose(fptr);
  free(B_c);
}

void save_uint_grayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                         float gamma, const char fname[]) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // the rank of the process
  unsigned long size = nx * ny;
  unsigned char *B_c = NULL;
  B_c = (unsigned char *)malloc(sizeof(unsigned char) * size);
  if (B_c == NULL) {
    printf(
        "\e[1;31mERROR: \e[0;37mno memory allocated to save uint grayscale \n");
    exit(1);
  }

  unsigned long k;
  float bmax = 0;

  for (k = 0; k < size; k++) {
    if (*(B + k) > bmax) {
      bmax = *(B + k);
    }
  }
  bmax = pow((float)bmax, 1 / gamma);

  for (k = 0; k < size; k++) {
    *(B_c + k) = pow((float)*(B + k), 1 / gamma) * 255 / bmax;
  }

  FILE *fptr;
  fptr = fopen(fname, "wb");
  fwrite(B_c, sizeof(unsigned char), size, fptr);
  fclose(fptr);
  free(B_c);
}

void parse(int argc, char *argv[], struct Param *param) {
  if (argc > 1) {
    *(*param).nx = atoi(argv[1]);
    *(*param).maxit = atoi(argv[2]);
    *(*param).minit = atoi(argv[3]);
    *(*param).D = (float)atoi(argv[4]);
    *(*param).depth = atoi(argv[5]);
  }
}

void export_param(struct Param param, const char fname[]) {
  FILE *fptr = NULL;
  fptr = fopen(fname, "w+");
  if (fptr == NULL) {
    printf("\e[1;31mERROR: \e[0;37mcannot create param file \n");
    exit(1);
  }
  fprintf(fptr,
          "nx=%u \nny=%u \nmaxit=%u \nminit=%u \nPoints per pixels=%.4f "
          "\ndepth=%u \n",
          *param.nx, *param.ny, *param.maxit, *param.minit, *param.D,
          *param.depth);
  fclose(fptr);
}
