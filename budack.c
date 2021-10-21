#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "budack.h"
#include <mpi.h>

double randomfloat(double min, double max);

void trajectories(
    int nx, int ny, double x_b[2],
    double y_b[2], unsigned char M_traj[ny][nx],
    long int maxtraj, int maxit, int minit);

int main()
{

    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    int nx=2000;
    long int N = 9000000;
    int maxit = 100;
    int minit = 0;


    int ny = 0;
    N = N/world_size; // dividing the work
    printf(" N per thread %ld \n",N );
    double a[2]={-2, 1.5}, b[2]={-1.5,1.5}, dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;
    printf("ok rank %d \n", rank);
    unsigned char M[ny][nx], M_sum[ny][nx];
    int arraysize[2]={ny, nx};

    //int (*M)[nx] = malloc(ny * nx);
    //int (*M_sum)[nx] = malloc(ny * nx);


    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);

    srand((unsigned int)time(NULL));
    trajectories(nx, ny, a, b, M, N, maxit, minit);

    MPI_Reduce(&M, &M_sum, nx*ny, MPI_UNSIGNED_CHAR, MPI_SUM, 0,
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
        fwrite(M_sum, sizeof(M_sum), 1, fp);

        fp = fopen("arraysize", "w+");
        if(fp == NULL)
        {
            printf("Error opening file\n");
            exit(1);
        }
        fwrite(arraysize, sizeof(arraysize), 1, fp);
        printf("DONE \n \n");
        }
    //free(M);
    //free(M_sum);
    MPI_Finalize(); // finish MPI environment
    return 0;
}
