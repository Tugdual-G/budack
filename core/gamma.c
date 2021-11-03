#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void save_uint_grayscale(unsigned int ny, unsigned int nx, float gamma,
                         char *fname_in, char *fname_out) {
  unsigned long size = nx * ny;

  unsigned int *rgb_chnl = (unsigned int *)malloc(sizeof(unsigned int) * size);
  FILE *fp;
  fp = fopen(fname_in, "rb");
  if (fp == NULL) {
    printf(" %s not found \n", fname_in);
  } else {
    fread(rgb_chnl, sizeof(unsigned int), size, fp);
    fclose(fp);

    unsigned char *rgb_chnl_nrmlzd;
    rgb_chnl_nrmlzd = (unsigned char *)malloc(sizeof(unsigned char) * size);

    unsigned long k;
    float bmax = 0;

    for (k = 0; k < size; k++) {
      if (*(rgb_chnl + k) > bmax) {
        bmax = *(rgb_chnl + k);
      }
    }
    bmax = pow((float)bmax, 1 / gamma);

    for (k = 0; k < size; k++) {
      *(rgb_chnl_nrmlzd + k) =
          pow((float)*(rgb_chnl + k), 1 / gamma) * 255 / bmax;
    }

    fp = fopen(fname_out, "wb");
    if (fp == NULL) {
      printf(" unable to write %s \n", fname_out);
    } else {
      fwrite(rgb_chnl_nrmlzd, sizeof(unsigned char), size, fp);
      fclose(fp);
    }
    free(rgb_chnl_nrmlzd);
  }
  free(rgb_chnl);
}

int main(int argc, char *argv[]) {
  MPI_Init(NULL, NULL); // initialize MPI environment
  int world_size;       // number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  int rank; // the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  unsigned int nx, ny;
  float gamma = 1.5;
  if (argc == 4) {
    nx = atoi(argv[1]);
    ny = atoi(argv[2]);
    gamma = atof(argv[3]);
  } else {
    printf("give correct number of arguments \n");
  }
  if (rank == 0) {
    save_uint_grayscale(ny, nx, gamma, "traj0.uint", "traj0gamma.char");
  } else if (rank == 1) {
    save_uint_grayscale(ny, nx, gamma, "traj1.uint", "traj1gamma.char");
  } else if (rank == 2) {
    save_uint_grayscale(ny, nx, gamma, "traj2.uint", "traj2gamma.char");
  }
  MPI_Finalize(); // finish MPI environment
  return 0;
}
