#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
                  unsigned int *M_traj2, long int D, int maxit, int minit,
                  double *M_brdr, unsigned int Nborder) {
  // Initialisation
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  double dx, x, y, x0, y0, x2, y2;

  // ntraj is the nuber of trajectories.
  // it is the number of iteration for a trajectory.
  long int ntraj = 0, it = 0, itraj = 0;
  unsigned int ij[maxit * 2 + 1], i;
  char diverge = 0;
  float current_D = 0;
  int maxit0 = minit + (maxit - minit) / 100.0,
      maxit1 = minit + (maxit - minit) / 10.0;

  dx = (x_b[1] - x_b[0]) / nx;
  while (current_D < D) {
    y0 = M_brdr[(2 * itraj) % Nborder] + gaussrand(dx);
    x0 = M_brdr[(2 * itraj + 1) % Nborder] + gaussrand(dx);

    it = 0;
    x = x0;
    y = y0;
    x2 = x * x;
    y2 = y * y;
    diverge = 0;

    /* // Here we test if the point is obviously in the set */
    /* // i.e. main cardioid and disk */
    /* q = (x-1/4)*(x-1/4)+y2; */
    /* if (q*(q+(x-1/4))<y2/5 || (x+1)*(x+1)+y2<1/17){ */
    /*   // If the point is in the set we do not iterate */
    /*   // and we search for another */
    /*   it = maxit; */
    /* } */
    ij[it * 2] = (y - y_b[0]) / dx;
    ij[it * 2 + 1] = (x - x_b[0]) / dx;

    while (it < maxit && diverge == 0) {
      // Storing the trajectories
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
      if (it < maxit0) {
        for (i = 0; i < it; i += 2) {
          *(M_traj0 + nx * ij[i] + ij[i + 1]) += 1;
        }
      } else if (it < maxit1) {
        for (i = 0; i < it; i += 2) {
          *(M_traj1 + nx * ij[i] + ij[i + 1]) += 1;
        }
      } else {
        ntraj++;
        for (i = 0; i < it; i += 2) {
          *(M_traj2 + nx * ij[i] + ij[i + 1]) += 1;
        }
      }
      if (rank == 0 && ntraj % 1000 == 0) {
        printf("\rPoints per pixel %-.4f/%1ld (core 0)", current_D, D);
        fflush(stdout);
      }
      current_D = (ntraj * dx * dx) / A;
    }
    itraj++;
  }
  if (rank == 0) {
    printf("\rPoints per pixel %-.4f/%1ld (core 0)\n", current_D, D);
  }
}

void border(unsigned int depth, long int Npts, double *M_brdr, unsigned char *M,
            unsigned int start, float a0, float b0, double dx,
            unsigned int nx) {
  // Return the list of the points at the boundary in index coordinates
  // relative to the subdomain bodaries.
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  FILE *fp;
  double x, y, x0, y0, x2, y2;
  unsigned int it = 0;
  unsigned int k = 0, n = 0;
  unsigned char mindepth = depth * 0.8;
  float sigma = 0.005;
  unsigned int i, j;
  unsigned int lenght_brdr = Lenght_strt;

  if (Npts <= lenght_brdr * 2) {
    lenght_brdr = Npts / 2;
  }
  start = lenght_brdr;

  fp = fopen("hint.double", "rb");
  if (fp == NULL) {
    if (rank == 0) {
      printf("'hint.double' not found, creating file : \n");
    }
    border_start(depth, M_brdr, M, lenght_brdr, a0, b0, dx, nx);
    k = lenght_brdr;
    if (rank == 0) {
      fp = fopen("hint.double", "wb");
      fwrite(M_brdr, sizeof(double), 2 * lenght_brdr, fp);
      printf(", written border points file \n");
      fclose(fp);
    }
  } else {
    fread(M_brdr, sizeof(double), 2 * lenght_brdr, fp);
    fclose(fp);
  }

  while (2 * k < Npts) {
    it = 0;
    n = n % start;
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

void save(char fname[], void *data, unsigned int size) {
  FILE *fp;
  fp = fopen(fname, "w+");
  if (fp == NULL) {
    printf("Error opening file %s, cannot save.\n", fname);
    exit(1);
  }
  fwrite(data, size, 1, fp);
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

void save_chargrayscale(unsigned int ny, unsigned int nx, unsigned int *B,
                        unsigned char weight, char fname[]) {
  int rank = MPI_Comm_rank(MPI_COMM_WORLD, &rank); // the rank of the process
  unsigned long size = nx * ny;
  unsigned char *B_c;
  B_c = (unsigned char *)malloc(sizeof(unsigned char) * size);

  unsigned long k;
  unsigned int bmax = 0;

  // mirror_traj(ny, nx, B);

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
  fptr = fopen(fname, "w+");
  fwrite(B_c, sizeof(unsigned char), size, fptr);
  fclose(fptr);
  free(B_c);
}
