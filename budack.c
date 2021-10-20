#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "budack.h"

double randomfloat(double min, double max);

void trajectories(
    int nx, int ny, double x_b[2],
    double y_b[2], unsigned char M_traj[ny][nx],
    long int maxtraj, int maxit);

int main()
{
    int nx=1000, ny=0;
    long int N = 9000000;
    double a[2]={-2, 1.5}, b[2]={-1.5,1.5}, dx;

    dx = (a[1]-a[0]) / (nx-1);
    ny = (b[1]-b[0])/dx;
    unsigned char M[ny][nx];

    printf("\nnx = %d ; ny = %d ; ny*nx= %d \n", nx, ny, ny*nx);

    srand((unsigned int)time(NULL));
    trajectories(nx, ny, a, b, M, N, 20);

    // Storing all trajetories on disk
    FILE *fp;
    fp = fopen("array", "w+");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(M, sizeof(M), 1, fp);
    printf("DONE \n \n");
    return 0;
}
