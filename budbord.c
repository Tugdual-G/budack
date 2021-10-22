#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "border.h"

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
    long int N = 10000000;
    unsigned int maxit = 20;
    unsigned int minit = 2;
    unsigned int ny = 0;
    unsigned int nx=500;
    float a[2]={-2, 1.1}, b[2]={-1.2,1.2}, dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;

    ////////////////////////////////////////////////
    //   Searching for points on the boundary
    ////////////////////////////////////////////////

    static unsigned int *M, *M_brdr;
    unsigned int Npts=nx*5;
    int arraysize[2]={ny, nx};
    unsigned char depth = 40;

    M_brdr = (unsigned int *)calloc(Npts, sizeof(unsigned int));
    M = (unsigned int *)calloc((int)nx*ny/2, sizeof(unsigned int));

    border(a, b[1], depth, nx, &Npts, M, M_brdr);
    int i;
    /* for (i=0; i<nx; i++) */
    /*     { */
    /*         printf("%u ", M_brdr[i] ); */
    /*     } */
    /* printf("\n \n \n"); */
    /* for (i=0; i<nx; i++) */
    /*     { */
    /*         printf("%u ", M[i] ); */
    /*     } */

    ////////////////////////////////////////////////
    //   Cumputing the trajectories
    ////////////////////////////////////////////////
    srand((unsigned int)time(NULL));
    unsigned int *B;
    B = (unsigned int *)calloc(nx*ny, sizeof(unsigned int));
    if (B==NULL){printf("Atention la");}
    printf("start computing traj \n");

    trajectories(nx, ny, a, b, B, N, maxit, minit, M_brdr, Npts);

    /* for (i=0; i<ny*nx; i++) */
    /*     { */
    /*         if (B[i] != 0) */
    /*             { */
    /*                 printf("%u ", M[i] ); */
    /*             } */
    /*     } */
    printf("\n \n \n");
    // Storing all trajetories on disk
    FILE *fp;
    fp = fopen("trajectories.o", "w+");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(B, sizeof(unsigned int)*nx*ny, 1, fp);

    fp = fopen("arraysize.o", "w+");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(arraysize, sizeof(arraysize), 1, fp);
    printf("Written trajectories.o on disk. \n  \n");

    printf("\e[?25h");
    free(B);
    free(M); free(M_brdr);
    return 0;
}
