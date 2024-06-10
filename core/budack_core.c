#include "budack_core.h"
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PI 3.141592654

double min3_double(double x, double y, double z) {
  double min = x < y ? x : y;
  min = min < z ? min : z;
  return min;
}
/* int max3_int(int x, int y, int z) { */
/*   int max = x < y ? y : x; */
/*   max = max < z ? z : max; */
/*   return max; */
/* } */

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

double randomdouble(double min, double max) {
  // Uniform distribution.
  double r, range = max - min;
  r = (double)rand() / (double)(RAND_MAX);
  r = min + r * range;
  return r;
}

void draw_trajectories(uint32_t *M, double x0, double y0, unsigned int nit,
                       double *x_b, double *y_b, unsigned int nx,
                       unsigned int ny) {
  // Locating the points in space
  // y for imaginary points
  // dx is the step of the grid
  double dx, x, y, x2, y2;
  dx = (x_b[1] - x_b[0]) / nx;

  // - it : number of iteration for one trajectory.
  int i = 0, j = 0;

  x = x0;
  y = y0;
  x2 = x * x;
  y2 = y * y;
  i = (y - y_b[0]) / dx;
  j = (x - x_b[0]) / dx;
  if (i >= 0 && i < (int)ny && j >= 0 && j < (int)nx) {
    *(M + nx * i + j) += 1;
  }
  for (unsigned int k = 0; k < nit; ++k) {
    y = 2 * x * y + y0;
    x = x2 - y2 + x0;
    x2 = x * x;
    y2 = y * y;
    i = (y - y_b[0]) / dx;
    j = (x - x_b[0]) / dx;
    if (i >= 0 && i < (int)ny && j >= 0 && j < (int)nx) {
      *(M + nx * i + j) += 1;
    }
  }
}

void trajectories(double D, int maxit, int minit, double *restrict starting_pts,
                  unsigned int length_strt, double dx) {
  // This where the trajectories are computed. Tree ranges of escape times are
  // used for rgb.
  // M_traj0 store the lowest escape time range of trajectories, and M_traj2
  // the highest.

  clock_t t0, t = 0;

  MPI_Request req = MPI_REQUEST_NULL;

  // For parallel processing
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  pts_msg sended_points[2 * PTS_MSG_SIZE];

  // Locating the points in space
  // y for imaginary points
  // dx is the step of the grid
  double x, y, x0, y0, x2, y2;

  // - itraj : number of trajectories satisfying the
  //           higgher minimal escape time.
  // - it : number of iteration for one trajectory.
  size_t itraj = 0;
  unsigned int it = 0;

  // - npts : total numbers of points visited for
  //          the higgest escape time range.
  unsigned int npts = 0;

  char diverge = 0;

  // Current min density.
  double density = 0.0;
  write_progress(density);
  // Definition of the 3 ranges of escape time,
  int maxit0 = minit + (maxit - minit) / 3.0,
      maxit1 = minit + 2 * (maxit - minit) / 3.0;

  double density_maxit = 0, density_minit = 0, density_medit = 0;

  while (density < 1.0) {
    // generate starting points
    y0 = starting_pts[(2 * itraj) % length_strt] + gaussrand(0.01);
    x0 = starting_pts[(2 * itraj + 1) % length_strt] + gaussrand(0.01);

    it = 0;
    x = x0;
    y = y0;
    x2 = x * x;
    y2 = y * y;
    diverge = 0;

    while (it < maxit && diverge == 0) {
      // Storing the trajectories
      y = 2 * x * y + y0;
      x = x2 - y2 + x0;
      x2 = x * x;
      y2 = y * y;
      ++it;
      if (x2 + y2 > 4) {
        diverge = 1;
      }
    }
    if (diverge == 1 && it > minit) {
      if (it < maxit0) {
        sended_points[npts].nit = it;
        sended_points[npts].color = 'r';
        sended_points[npts].x = x0;
        sended_points[npts].y = y0;
        density_minit += (it * dx * dx) / (AREA * D);
        maxit0 = density_minit < 1.0 ? maxit0 : 0;

      } else if (it < maxit1) {
        sended_points[npts].nit = it;
        sended_points[npts].color = 'g';
        sended_points[npts].x = x0;
        sended_points[npts].y = y0;
        density_medit += (it * dx * dx) / (AREA * D);
        maxit1 = density_medit < 1.0 ? maxit1 : maxit0;

      } else {
        sended_points[npts].nit = it;
        sended_points[npts].color = 'b';
        sended_points[npts].x = x0;
        sended_points[npts].y = y0;
        ++itraj;
        density_maxit += (it * dx * dx) / (AREA * D);
        maxit = density_maxit < 1.0 ? maxit : maxit1;
      }
      density = min3_double(density_minit, density_medit, density_maxit);
      ++npts;
      if (npts % PTS_MSG_SIZE == 0) {
        if (rank == 1) {
          write_progress(density);
        }
        t0 = clock();
        MPI_Wait(&req, MPI_STATUS_IGNORE);
        t += clock() - t0;
        MPI_Isend(sended_points + npts - PTS_MSG_SIZE,
                  sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank,
                  MPI_COMM_WORLD, &req);
        /* MPI_Bsend(sended_points + npts - PTS_MSG_SIZE, */
        /*           sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank, */
        /*           MPI_COMM_WORLD); */
        npts %= 2 * PTS_MSG_SIZE;
      }
    }
  }

  if (npts % PTS_MSG_SIZE != 0) {
    for (unsigned char i = npts; i < PTS_MSG_SIZE * 2; ++i) {
      sended_points[i].color = 0;
    }
    if (rank == 1) {
      write_progress(density);
    }
    MPI_Wait(&req, MPI_STATUS_IGNORE);
    MPI_Isend(sended_points + PTS_MSG_SIZE * (npts / PTS_MSG_SIZE),
              sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank, MPI_COMM_WORLD,
              &req);
    /* MPI_Bsend(sended_points + PTS_MSG_SIZE * (npts / PTS_MSG_SIZE), */
    /*           sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank, */
    /*           MPI_COMM_WORLD); */
  }
  // Sending a flag (0)
  sended_points[0].color = 0;
  MPI_Wait(&req, MPI_STATUS_IGNORE);
  MPI_Isend(sended_points, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank,
            MPI_COMM_WORLD, &req);
  printf("waiting time rank %i : %lf s \n", rank, (double)t / CLOCKS_PER_SEC);
  /* MPI_Bsend(sended_points, sizeof(pts_msg) * PTS_MSG_SIZE, MPI_BYTE, 0, rank,
   */
  /*           MPI_COMM_WORLD); */
}

int border(unsigned int depth, long int length_strt,
           double *restrict starting_pts) {
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
    exit(1);
  }

  // The depth is rounded to the nearest multiple of 20
  depth = ((depth - 11) / 20 + 1) * 20;
  strcpy(filename, "core/hints");
  snprintf(depth_str, 9, "%u", depth);
  strncat(filename, depth_str, 99);
  strncat(filename, ".double", 99);
  int *displacement = NULL;
  int *countrecv = NULL;

  FILE *fp = NULL;
  fp = fopen(filename, "rb");
  // If we can't find the file we create it.
  if (fp == NULL) {
    int send_length = 2 * length_strt;
    double *ALL_pts = NULL;
    if (rank == 0) {
      starting_pts = MPI_IN_PLACE;
      displacement = (int *)malloc(sizeof(int) * world_size);
      countrecv = (int *)malloc(sizeof(int) * world_size);
      if (!displacement | !countrecv) {
        printf("Error border \n");
        exit(1);
      }
      displacement[0] = 0;
      countrecv[0] = 0;
      for (int i = 1; i < world_size; ++i) {
        displacement[i] = send_length * (i - 1);
        countrecv[i] = send_length;
      };
      send_length = 0;

      printf("%s not found, creating file : \n", filename);

      ALL_pts =
          (double *)malloc(length_strt * 2 * (world_size - 1) * sizeof(double));

      if (ALL_pts == NULL) {
        printf("\e[1;31mERROR: \e[0;37mno memory allocated to save starting "
               "pts \n");
        exit(1);
      }

      MPI_Gatherv(starting_pts, send_length, MPI_DOUBLE, ALL_pts, countrecv,
                  displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);

      free(countrecv);
      free(displacement);
      printf("saving hints \n");
      putc('_', stdout);
      save(filename, ALL_pts, sizeof(double),
           (world_size - 1) * 2 * length_strt);
      free(ALL_pts);

    } else {
      putc('_', stdout);
      fflush(stdout);
      border_start(depth, starting_pts, length_strt);
      MPI_Gatherv(starting_pts, send_length, MPI_DOUBLE, ALL_pts, countrecv,
                  displacement, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    return 1;
  }
  if (rank != 0) {
    // If we find the file we load the points.
    fseek(fp, length_strt * sizeof(double) * 2 * (rank - 1), SEEK_SET);
    fread(starting_pts, sizeof(double), 2 * length_strt, fp);
  }
  fclose(fp);
  return 0;
}

void border_start(unsigned int depth, double *starting_pts,
                  unsigned int length_start) {
  // Found random points close to the border of the Mdlbrt set
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  double x, y, x0, y0, x2, y2;
  unsigned int it = 0;
  unsigned int k = 0;
  unsigned char mindepth = depth * 0.8;
  fflush(stdout);
  while (k < 2 * length_start) {
    it = 0;

    x0 = randomdouble(-2, 0.5);
    y0 = randomdouble(0, 1.5);

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
      *(starting_pts + k) = y0;
      *(starting_pts + k + 1) = x0;
      k += 2;
      /* if (rank == 1) { */
      /*   printf("Generating file : %u / %u", k, length_start); */
      /*   fflush(stdout); */
      /* } */
    }
  }
  /* if (rank == 1) { */
  /*   printf("Generating file : %u / %u", k, length_start); */
  /*   printf("\n"); */
  /*   fflush(stdout); */
  /* } */
  printf("Generated points rank %i \n", rank);
  fflush(stdout);
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

void parse(int argc, char *argv[], Param *param) {
  if (argc > 1) {
    *(*param).nx = atoi(argv[1]);
    *(*param).maxit = atoi(argv[2]);
    *(*param).minit = atoi(argv[3]);
    *(*param).D = (double)atoi(argv[4]);
    *(*param).depth = atoi(argv[5]);
    (*param).output_dir = argv[6];
  }
  if (strlen((*param).output_dir) > MAX_PATH_LENGTH) {
    printf("\e[1;31mERROR: \e[0;37 output directory path is more than 490 "
           "characters.\n");
    exit(1);
  }
}

void export_param(Param param) {
  FILE *fptr = NULL;
  char filename[MAX_PATH_LENGTH + 21] = {'\0'};
  unsigned outdir_str_len = strlen(param.output_dir);
  strncpy(filename, param.output_dir, MAX_PATH_LENGTH);
  if (param.output_dir[outdir_str_len - 1] != '/') {
    filename[outdir_str_len] = '/';
    outdir_str_len += 1;
  }
  strncat(filename, "param.txt", 10);

  fptr = fopen(filename, "w+");
  if (fptr == NULL) {
    printf("\e[1;31mERROR: \e[0;37mcannot create param file : %s \n", filename);
    exit(1);
  }
  fprintf(fptr,
          "nx=%u \nny=%u \nmaxit=%u \nminit=%u \nPoints per pixels=%.4f "
          "\ndepth=%u \n",
          *param.nx, *param.ny, *param.maxit, *param.minit, *param.D,
          *param.depth);
  fclose(fptr);
}

void write_progress(double density) {
  FILE *fptr = NULL;
  fptr = fopen("/tmp/progress", "w+");
  if (fptr == NULL) {
    printf("\e[1;31mERROR: \e[0;37mcannot create progress file \n");
    exit(1);
  }
  if (density > -1.0) {
    fprintf(fptr, "points density %-.4f/100", 100 * density);
  } else {
    fprintf(fptr, "0");
  }
  fclose(fptr);
}
