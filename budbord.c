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
    unsigned int maxit = 2000;
    unsigned int minit = 5;
    unsigned int ny = 0;
    unsigned int nx=500;
    float a[2]={-2.3, 1.3}, b[2]={-1.5,1.5}, dx;

    printf("\e[?25l");
    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;



    ////////////////////////////////////////////////
    //   Searching for points on the boundary
    ////////////////////////////////////////////////

    clock_t begin = clock();

    static unsigned int *M, *M_brdr;
    unsigned int Npts=nx*10;
    int arraysize[2]={ny, nx};
    unsigned char depth = 40;

    M_brdr = (unsigned int *)calloc(Npts, sizeof(unsigned int));
    M = (unsigned int *)calloc((int)nx*ny/2, sizeof(unsigned int));
    if (M==NULL){printf("Error, no memory space allocated for computing"); return 0;}

    border(a, b[1], depth, nx, &Npts, M, M_brdr);
    int i;


    clock_t end = clock();
    float t_comp = (float)(end - begin);
    t_comp = t_comp/CLOCKS_PER_SEC;
    printf("Time elapsed searching for border points %f s \n", t_comp);
    printf("Found %u points \n", Npts);

    ////////////////////////////////////////////////
    //   Cumputing the trajectories
    ////////////////////////////////////////////////

    srand((unsigned int)time(NULL));
    unsigned int *B;
    B = (unsigned int *)calloc(nx*ny, sizeof(unsigned int));
    if (B==NULL){printf("Error, no memory space allocated for computing"); return 0;}

    begin = clock();

    trajectories(nx, ny, a, b, B, N, maxit, minit, M_brdr, Npts);

    end = clock();
    t_comp = (float)(end - begin);
    t_comp = t_comp/CLOCKS_PER_SEC;
    printf("\nTime elapsed computing trajectories %f s \n", t_comp);

    // Storing variables on disk
    save("trajectories.data", B, sizeof(unsigned int)*nx*ny);
    save("arraysize.data", arraysize, sizeof(arraysize));
    save("boundary.data", M_brdr, sizeof(unsigned int)*Npts);

    printf("\e[?25h"); // display the cursor again
    free(M); free(M_brdr); free(B);
    return 0;
}
