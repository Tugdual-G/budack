#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
double poly(double gamma, double x) {
  // gamma != 1
  double b = 256 * (gamma - 8) / (gamma - 1);
  double c = (8 - gamma) / 7.0;
  return pow(x, gamma) / c + 256 * pow(x, 8) / b;
}

void recti_save_to_uchar(unsigned int ny, unsigned int nx, double gamma,
                         char *fname_in, char *fname_out) {
  if (gamma == 1.0) {
    printf("Error in recti_save_to_uchar : gamma should not be equal to 1 \n");
    exit(1);
  }
  unsigned long size = nx * ny;

  unsigned int *rgb_chnl = (unsigned int *)malloc(sizeof(unsigned int) * size);
  FILE *fp;
  fp = fopen(fname_in, "rb");
  if (fp == NULL) {
    printf(" %s not found \n", fname_in);
    exit(1);
  }
  if (fread(rgb_chnl, sizeof(unsigned int), size, fp) != size) {
    printf("Error, cannot read the whole input file.");
    fclose(fp);
    exit(1);
  };
  fclose(fp);

  unsigned char *rgb_chnl_nrmlzd;
  rgb_chnl_nrmlzd = (unsigned char *)malloc(sizeof(unsigned char) * size);

  unsigned long k;
  double bmax = 0;

  for (k = 0; k < size; k++) {
    if (*(rgb_chnl + k) > bmax) {
      bmax = *(rgb_chnl + k);
    }
  }

  for (k = 0; k < size; k++) {
    *(rgb_chnl_nrmlzd + k) =
        (unsigned char)255.0 * poly(gamma, (double)*(rgb_chnl + k) / bmax);
  }

  fp = fopen(fname_out, "wb");
  if (fp == NULL) {
    printf(" unable to write %s \n", fname_out);
    exit(1);
  }

  fwrite(rgb_chnl_nrmlzd, sizeof(unsigned char), size, fp);
  fclose(fp);
  free(rgb_chnl_nrmlzd);
  free(rgb_chnl);
}
void gamma_save_to_uchar(unsigned int ny, unsigned int nx, double gamma,
                         char *fname_in, char *fname_out) {
  unsigned long size = nx * ny;

  unsigned int *rgb_chnl = (unsigned int *)malloc(sizeof(unsigned int) * size);
  FILE *fp;
  fp = fopen(fname_in, "rb");
  if (fp == NULL) {
    printf(" %s not found \n", fname_in);
    exit(1);
  }
  fread(rgb_chnl, sizeof(unsigned int), size, fp);
  fclose(fp);

  unsigned char *rgb_chnl_nrmlzd;
  rgb_chnl_nrmlzd = (unsigned char *)malloc(sizeof(unsigned char) * size);

  unsigned long k;
  double bmax = 0;

  for (k = 0; k < size; k++) {
    if (*(rgb_chnl + k) > bmax) {
      bmax = *(rgb_chnl + k);
    }
  }
  bmax = pow((double)bmax, 1 / gamma);

  for (k = 0; k < size; k++) {
    *(rgb_chnl_nrmlzd + k) =
        pow((double)*(rgb_chnl + k), 1 / gamma) * 255.0 / bmax;
  }

  fp = fopen(fname_out, "wb");
  if (fp == NULL) {
    printf(" unable to write %s \n", fname_out);
    exit(1);
  }

  fwrite(rgb_chnl_nrmlzd, sizeof(unsigned char), size, fp);
  fclose(fp);
  free(rgb_chnl_nrmlzd);
  free(rgb_chnl);
}

int main(int argc, char *argv[]) {
  MPI_Init(NULL, NULL); // initialize MPI environment
  int world_size;       // number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  unsigned int nx, ny;
  double gamma = 1.5;
  size_t path_length = 500;
  char *path_to_infile = (char *)malloc((1 + path_length) * sizeof(char));
  char *path_to_outfile = (char *)malloc((1 + path_length) * sizeof(char));
  path_to_infile[path_length] = '\0';
  path_to_outfile[path_length] = '\0';
  char rect = 0;

  if (argc == 7) {
    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
    gamma = (double)atof(argv[3]);
    strncpy(path_to_infile, argv[4], path_length);
    strncpy(path_to_outfile, argv[5], path_length);
    rect = (char)atoi(argv[6]);

  } else {
    printf("Please provide the correct number of arguments: nx ny gamma infile "
           "outfile rect \n");
    exit(1);
  }
  if (rect) {
    if (rank == 0) {
      strncat(path_to_infile, "traj0.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj0gamma.char",
              path_length - strlen(path_to_outfile));

      recti_save_to_uchar(ny, nx, 1 / gamma, path_to_infile, path_to_outfile);
    } else if (rank == 1) {
      strncat(path_to_infile, "traj1.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj1gamma.char",
              path_length - strlen(path_to_outfile));

      recti_save_to_uchar(ny, nx, 1 / gamma, path_to_infile, path_to_outfile);
    } else if (rank == 2) {
      strncat(path_to_infile, "traj2.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj2gamma.char",
              path_length - strlen(path_to_outfile));

      recti_save_to_uchar(ny, nx, 1 / gamma, path_to_infile, path_to_outfile);
    }

  } else {
    if (rank == 0) {
      strncat(path_to_infile, "traj0.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj0gamma.char",
              path_length - strlen(path_to_outfile));

      gamma_save_to_uchar(ny, nx, gamma, path_to_infile, path_to_outfile);
    } else if (rank == 1) {
      strncat(path_to_infile, "traj1.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj1gamma.char",
              path_length - strlen(path_to_outfile));

      gamma_save_to_uchar(ny, nx, gamma, path_to_infile, path_to_outfile);
    } else if (rank == 2) {
      strncat(path_to_infile, "traj2.uint",
              path_length - strlen(path_to_infile));
      strncat(path_to_outfile, "traj2gamma.char",
              path_length - strlen(path_to_outfile));

      gamma_save_to_uchar(ny, nx, gamma, path_to_infile, path_to_outfile);
    }
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
