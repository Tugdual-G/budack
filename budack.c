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
    long int N = 1*e6;
    unsigned int nx= 1*e3;
    unsigned int maxit = 1000;
    ////////////////////////////////////////////////


    unsigned int minit = 5;
    unsigned int ny = 0;
    float a[2]={-2.3, 1.3}, b[2]={-1.5,1.5}, dx;

    printf("\e[?25l");
    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;

    if (rank==0){
        printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);
        printf("maxit = %d ; minit = %d \n", maxit, minit);
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
    unsigned char depth = 40;

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

    int arraysize[2]={ny, nx};
    if (rank==0)
        {
            end = clock();
            t_comp = (float)(end - begin);
            t_comp = t_comp/CLOCKS_PER_SEC;
            printf("\e[?25h"); // display the cursor again
            printf("\nTime elapsed computing trajectories %f s \n", t_comp);

            // Storing variables on disk
            save("trajectories.data", B_sum, sizeof(unsigned int)*nx*ny);
            save("arraysize.data", arraysize, sizeof(arraysize));
            save("boundary.data", M_brdr, sizeof(unsigned int)*Nborder);

        }
    //MPI_Barrier(MPI_COMM_WORLD);
    //free(M);
    //printf("free M \n");
    //MPI_Barrier(MPI_COMM_WORLD);
    ////free(M_brdr);  // WTF is that bug !!!
    //printf("free M_brdr \n");
    //MPI_Barrier(MPI_COMM_WORLD);
    //free(B);
    //printf("free B \n");
    //MPI_Barrier(MPI_COMM_WORLD);
    //free(B_sum);
    //printf("free B_sum \n");
    MPI_Finalize(); // finish MPI environment
    return 0;
}
