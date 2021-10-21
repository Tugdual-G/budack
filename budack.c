#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "budack.h"
#include <mpi.h>


int main()
{

    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    int nx=2000;
    long int N = 9000000/world_size;
    unsigned int maxit = 100;
    unsigned int minit = 0;


    int ny = 0;
    double a[2]={-2, 1.5}, b[2]={-1.5,1.5}, dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;
    int *M, *M_sum;
    int arraysize[2]={ny, nx};
    M = zeros(ny, nx);
    M_sum = zeros(ny, nx);

    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);

    srand((unsigned int)time(NULL));

    trajectories(nx, ny, a, b, M, N, maxit, minit);

    MPI_Reduce(M, M_sum, nx*ny, MPI_INT, MPI_SUM, 0,
        MPI_COMM_WORLD);

    if (rank==0){
        // Storing all trajetories on disk
        FILE *fp;
        fp = fopen("array", "w+");
        if(fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        fwrite(M_sum, sizeof(int)*nx*ny, 1, fp);

        fp = fopen("arraysize", "w+");
        if(fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        fwrite(arraysize, sizeof(arraysize), 1, fp);
        printf("DONE \n \n");
        }
    /* free(M); */
    /* free(M_sum); */
    MPI_Finalize(); // finish MPI environment
    return 0;
}
