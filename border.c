#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "border.h"

const int e6 = 1000000;
const int e3 = 1000;

int main()
{

    unsigned int nx=1000;
    unsigned int N=nx*10;
    unsigned char depth = 40;
    unsigned int ny = 0;
    float a[2]={-2, 1}, b[2]={-1.5,1.5};
    double dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = 1+(b[1]-b[0])/dx;
    int *M;
    int *M_border;
    M_border = zeros(1, N+nx*ny/2);
    M = M_border+N;

    border(a, b[1], depth, nx, &N, M, M_border);
    unsigned int i;

    // Storing all trajectories on disk
    FILE *fp;
    fp = fopen("M.o", "w+");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(M, sizeof(int)*(int)(ny/2)*nx, 1, fp);
    printf("Written M.o on disk. \n  \n");
    fp = fopen("M_b.o", "w+");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(M_border, sizeof(int)*N, 1, fp);
    printf("N=%d \n", N/2);
    printf("\e[?25h");

    return 0;
}
