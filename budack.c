#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "budack.h"
#include <mpi.h>

const int e6 = 1000000;
const int e3 = 1000;

struct Param
{
    long int N;
    int nx, ny;
    int maxit;
    int minit;
    double a[2];
    double b[2];
    int *M;
};

int main()
{

    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    int nx=500;
    long int N = 10*e6;
    unsigned int maxit = 20;
    unsigned int minit = 0;

    int ny = 0;
    double a[2]={-2, 1.1}, b[2]={-1.2,1.2}, dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;
    int *M, *M_sum;
    int arraysize[2]={ny, nx};
    M = zeros(ny, nx);
    M_sum = zeros(ny, nx);

    if (rank==0){
        printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);
        printf("maxit = %d ; minit = %d \n", maxit, minit);
        printf("Computing %ld trajectories\n", N);
        printf("Begin computation on %d cores \n", world_size);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    srand((unsigned int)time(NULL));

    clock_t begin = clock();

    N = N/world_size;
    trajectories(nx, ny, a, b, M, N, maxit, minit);

    MPI_Reduce(M, M_sum, nx*ny, MPI_INT, MPI_SUM, 0,
        MPI_COMM_WORLD);

    clock_t end = clock();
    float t_comp = (float)(end - begin);
    t_comp = t_comp/CLOCKS_PER_SEC;

    if (rank==0){
        // Storing all trajetories on disk
        printf("\nComputing time %f s\n", t_comp);
        FILE *fp;
        fp = fopen("trajectories.o", "w+");
        if(fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        fwrite(M_sum, sizeof(int)*nx*ny, 1, fp);

        fp = fopen("arraysize.o", "w+");
        if(fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        fwrite(arraysize, sizeof(arraysize), 1, fp);
        printf("Written trajectories.o on disk. \n  \n");
        }
    /* free(M); */
    /* free(M_sum); */
    MPI_Finalize(); // finish MPI environment
    return 0;
}
