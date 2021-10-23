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
    unsigned int nx, ny;
    unsigned int maxit;
    unsigned int minit;
    float a[2];
    float b[2];
    unsigned int *B;
    unsigned int *B_sum;
    unsigned int *M;
    unsigned int *M_brdr;
    /* char B_sum_nm[]; */
    /* char M_nm[]; */
    /* char M_brdr_nm[]; */
};


int main()
{
    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    ////////////////////////////////////////////////
    //   Most important parameters
    ////////////////////////////////////////////////
    long int N = 10*e6;
    unsigned int nx= 1*e3;
    unsigned int maxit = 20;
    ////////////////////////////////////////////////


    unsigned int minit = 1;
    unsigned int ny = 0;
    float a[2]={-2.3, 1.3}, b[2]={-1.5,1.5}, dx;

    // x and y are discretized at the midle of the cells
    dx = (a[1]-a[0]) / nx;
    ny = 2*b[1]/dx;

    if (rank==0){
        printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);
        printf("maxit = %d ; minit = %d \n", maxit, minit);
        printf("Depth of the data written to disk : %lu \n \n", sizeof(unsigned int));
        printf("Computing %ld trajectories\n", N);
        printf("Begin computation on %d cores \n", world_size);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    ////////////////////////////////////////////////
    //   Searching for points on the boundary
    ////////////////////////////////////////////////

    clock_t begin = clock();

    unsigned int *M, *M_brdr;
    unsigned int Nborder=nx*ny/20;
    unsigned char depth = 90;

    M_brdr = (unsigned int *)calloc(Nborder, sizeof(unsigned int));
    M = (unsigned int *)calloc((int)nx*ny/2, sizeof(unsigned int));
    if (M==NULL){printf("Error, no memory space allocated for computing"); return 0;}

    border(a, b[1], depth, nx, &Nborder, M, M_brdr);

    clock_t end = clock();
    float t_comp = (float)(end - begin);
    t_comp = t_comp/CLOCKS_PER_SEC;
    if (rank==0)
        {
            printf("Time elapsed searching for border points %f s \n", t_comp);
            printf("Core 0 found %u border points \n", Nborder);
        }

    ////////////////////////////////////////////////
    //   Cumputing the trajectories
    ////////////////////////////////////////////////

    srand((unsigned int)time(NULL));
    unsigned int *B, *B_sum;
    B = (unsigned int *)calloc(nx*ny, sizeof(unsigned int));
    B_sum = (unsigned int *)calloc(nx*ny, sizeof(unsigned int));
    N = (unsigned int) N/world_size;
    if (B_sum==NULL){printf("Error, no memory space (heap) allocated for storing results"); return 0;}

    if (rank==0){begin = clock();}

    trajectories(nx, ny, a, b, B, N, maxit, minit, M_brdr, Nborder);

    MPI_Reduce(B, B_sum, nx*ny, MPI_INT, MPI_SUM, 0,
        MPI_COMM_WORLD);

    unsigned int arraysize[2]={ny, nx};

    if (rank==0)
        {
            end = clock();
            t_comp = (float)(end - begin);
            t_comp = t_comp/CLOCKS_PER_SEC;
            printf("\nTime elapsed computing trajectories %f s \n", t_comp);

            // Storing variables on disk
            save("arraysize.uint", arraysize, sizeof(arraysize));
            save("boundary.uint", M_brdr, sizeof(unsigned int)*Nborder);
            save_chargrayscale(ny,nx,B_sum,"trajectories.char");
        }

    free(M);
    free(M_brdr);
    free(B);
    free(B_sum);
    if (rank==0){printf("allocated space freed \n");}
    MPI_Finalize(); // finish MPI environment
    return 0;
}
