#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PI 3.141592654
const long int A = 9;
const unsigned int Lenght_strt = 50000;

void border_start(unsigned int depth, double *M_brdr, unsigned char *M,
                  unsigned int start, float a0, float b0, double dx,
                  unsigned int nx);

double gaussrand(double dx) {
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
  double r, range = max - min;
  r = (double)rand() / (double)(RAND_MAX);
  r = min + r * range;
  return r;
}

void trajectories(unsigned int nx, unsigned int ny, float x_b[2], float y_b[2],
                  unsigned int *M_traj0, unsigned int *M_traj1,
                  unsigned int *M_traj2, float D, int maxit, int minit,
                  double *M_brdr, unsigned int Nborder) {
  // Initialisation
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  double dx, x, y, x0, y0, x2, y2;

  // ntraj is the nuber of trajectories.
  // it is the number of iteration for a trajectory.
  long unsigned int it = 0, itraj = 0;
  long long unsigned int npts = 0;
  unsigned int ij[maxit * 2 + 1], i;
  char diverge = 0;
  float current_D = 0;
  int maxit0 = minit + (maxit - minit) / 3.0,
      maxit1 = minit + 2 * (maxit - minit) / 3.0;
  unsigned char n0, n1;
  unsigned char n0_max = maxit / maxit0, n1_max = maxit / maxit1;

  dx = (x_b[1] - x_b[0]) / nx;
  while (current_D < D) {
    y0 = M_brdr[(2 * itraj) % Nborder] + gaussrand(0.01);
    x0 = M_brdr[(2 * itraj + 1) % Nborder] + gaussrand(0.01);

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
      if (x_b[0] < x && x < x_b[1] && y_b[0] < y && y < y_b[1]) {
        ij[it * 2] = (y - y_b[0]) / dx;
        ij[it * 2 + 1] = (x - x_b[0]) / dx;
      }
      if (x2 + y2 > 4) {
        diverge = 1;
      }
    }
    if (diverge == 1 && it > minit) {
      if (it < maxit0 && n0 < n0_max) {
        n0++;
        for (i = 0; i < it; i += 2) {
          *(M_traj0 + nx * ij[i] + ij[i + 1]) += 1;
        }
      } else if (it < maxit1 && n1 < n1_max) {
        n1++;
        for (i = 0; i < it; i += 2) {
          *(M_traj1 + nx * ij[i] + ij[i + 1]) += 1;
        }
      } else {
        for (i = 0; i < it; i += 2) {
          *(M_traj2 + nx * ij[i] + ij[i + 1]) += 1;
        }
        itraj++;
        npts += it;
        current_D = (npts * dx * dx) / A;
      }
      if (rank == 0 && itraj % 10 == 0) {
        printf("\rPoints per pixel %-.4f/%.4f (core 0)", current_D, D);
        fflush(stdout);
      }
    }
  }
  if (rank == 0) {
    printf("\rPoints per pixel %-.4f/%.4f (core 0)\n", current_D, D);
  }
}

void border(unsigned int depth, long int Npts, double *M_brdr, unsigned char *M,
            unsigned int start, float a0, float b0, double dx,
            unsigned int nx) {
  // Return the list of the points at the boundary in index coordinates
  // relative to the subdomain bodaries.
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  FILE *fp = NULL;
  double x, y, x0, y0, x2, y2;
  unsigned int it = 0;
  unsigned int k = 0, n = 0;
  unsigned char mindepth = depth * 0.8;
  float sigma = 0.005;
  unsigned int i, j;
  unsigned int lenght_brdr = Lenght_strt;
  char filename[24];
  if (depth <= 50) {
    strcpy(filename, "core/hints50.double");
  } else if (depth > 50 && depth <= 100) {
    strcpy(filename, "core/hints100.double");
  } else if (depth > 100 && depth <= 1000) {
    strcpy(filename, "core/hints1000.double");
  } else if (depth > 1000 && depth <= 10000) {
    strcpy(filename, "core/hints10000.double");
  } else if (depth > 10000) {
    strcpy(filename, "core/hints100000.double");
  }

  if (Npts <= lenght_brdr * 2) {
    lenght_brdr = Npts / 2;
  }
  start = lenght_brdr;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    if (rank == 0) {
      printf("%s not found, creating file : \n", filename);
    }
    border_start(depth, M_brdr, M, Lenght_strt, a0, b0, dx, nx);
    k = lenght_brdr;
    if (rank == 0) {
      fp = fopen(filename, "wb");
      if (fp == NULL) {
        printf(" Error, cannot create %s \n", filename);
        exit(1);
      }
      fwrite(M_brdr, sizeof(double), 2 * Lenght_strt, fp);
      printf(", written border points file \n");
      fclose(fp);
    }
  } else {
    fread(M_brdr, sizeof(double), 2 * lenght_brdr, fp);
    fclose(fp);
  }

  while (2 * k < Npts) {
    it = 0;
    n = n % Lenght_strt;
    x0 = *(M_brdr + n * 2 + 1) + gaussrand(sigma);
    y0 = *(M_brdr + n * 2) + gaussrand(sigma);
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
      *(M_brdr + k * 2) = y0;
      *(M_brdr + k * 2 + 1) = x0;
      i = (y0 - b0) / dx;
      j = (x0 - a0) / dx;
      *(M + i * nx + j) = 255;
      k++;
      n++;
      if (rank == 0) {
        printf("\rGenerating starting point : %u / %ld", k, Npts / 2);
        fflush(stdout);
      }
    }
  }
  if (rank == 0) {
    printf("\n");
  }
}

void border_start(unsigned int depth, double *M_brdr, unsigned char *M,
                  unsigned int start, float a0, float b0, double dx,
                  unsigned int nx) {
  // Return the list of the points at the boundary in index coordinates
  // relative to the subdomain bodaries.
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
    if (x0 < -0.75) {
      y0 = randomfloat(0, 0.5);
    } else {
      y0 = randomfloat(0, 1.5);
    }

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
      *(M_brdr + k * 2) = y0;
      *(M_brdr + k * 2 + 1) = x0;
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

void save(char fname[], void *data, unsigned int size,
          unsigned int n_elements) {
  FILE *fp = NULL;
  fp = fopen(fname, "wb");
  if (fp == NULL) {
    printf("Error opening file %s, cannot save.\n", fname);
    exit(1);
  }
  fwrite(data, size, n_elements, fp);
  fclose(fp);
}

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B) {
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
                         unsigned char weight, char fname[]) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // the rank of the process
  unsigned long size = nx * ny;
  unsigned char *B_c = NULL;
  B_c = (unsigned char *)malloc(sizeof(unsigned char) * size);
  if (B_c == NULL) {
    printf("Error no memory allocated to save char grayscale \n");
    exit(1);
  }

  unsigned long k;
  unsigned int bmax = 0;

  for (k = 0; k < size; k++) {
    if (*(B + k) > bmax) {
      bmax = *(B + k);
    }
  }
  printf("Maximum accumulted points %u , rank %d \n", bmax, rank);
  for (k = 0; k < size; k++) {
    *(B_c + k) = (double)*(B + k) * 255 / (weight * bmax);
  }

  FILE *fptr;
  fptr = fopen(fname, "wb");
  fwrite(B_c, sizeof(unsigned char), size, fptr);
  fclose(fptr);
  free(B_c);
}

void save_uint_grayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                         float gamma, char fname[]) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // the rank of the process
  unsigned long size = nx * ny;
  unsigned char *B_c = NULL;
  B_c = (unsigned char *)malloc(sizeof(unsigned char) * size);
  if (B_c == NULL) {
    printf("Error no memory allocated to save uint grayscale \n");
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

struct Param {
  unsigned int *nx, *ny, *maxit, *minit, *depth;
  float *D;
};

void parse(int argc, char *argv[], struct Param *param) {
  if (argc > 1) {
    *(*param).nx = atoi(argv[1]);
    *(*param).maxit = atoi(argv[2]);
    *(*param).minit = atoi(argv[3]);
    *(*param).D = (float)atoi(argv[4]);
    *(*param).depth = atoi(argv[5]);
  }
}
void export_param(struct Param param, char filename[]) {
  FILE *fptr = NULL;
  fptr = fopen(filename, "w+");
  if (fptr == NULL) {
    printf(" Error cannot creat param file \n");
    exit(1);
  }
  fprintf(fptr,
          "nx=%u \nny=%u \nmaxit=%u \nminit=%u \nPoints per pixels=%.4f "
          "\ndepth=%u \n",
          *param.nx, *param.ny, *param.maxit, *param.minit, *param.D,
          *param.depth);
  fclose(fptr);
}

void cd_to_root_dir(char *arg0) {
  // This procedure is only here to hide an realy ugly
  // part of code.
  char *cwd = NULL;
  cwd = (char *)malloc(500 * sizeof(char));
  if (cwd == NULL) {
    printf(" Cannot find working directory \n");
    exit(1);
  }
  strncpy(cwd, arg0, strlen(arg0) - 6 * sizeof(char));
  char *rightdir = NULL;
  rightdir = strstr(cwd, "core");
  if (rightdir == NULL && strlen(cwd) != 0) {
    printf(" oulasavapa \n");
    exit(0);
  } else if (strlen(cwd) != 0) {
    chdir(cwd);
  }
  chdir("..");
  free(cwd);
}
