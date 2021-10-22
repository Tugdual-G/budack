#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <math.h>


#define PI 3.141592654

double gaussrand(double dx)
{
  static double U, V;
  static int phase = 0;
  double Z;

  if(phase == 0) {
    U = (rand() + 1.) / (RAND_MAX + 2.);
    V = rand() / (RAND_MAX + 1.);
    Z = sqrt(-2 * log(U)) * sin(2 * PI * V);
  } else
    Z = sqrt(-2 * log(U)) * cos(2 * PI * V);

  phase = 1 - phase;

  return Z*dx*2;
}

double randomfloat(double min, double max)
  {
    double r, range=max-min;
    r = (double)rand()/(double)(RAND_MAX);
    r = min + r * range;
    return r;
  }

void trajectories(
    unsigned int nx,
    unsigned int ny,
    float x_b[2],
    float y_b[2],
    int *M_traj,
    long int maxtraj,
    int maxit,
    int minit,
    unsigned int *M_border,
    unsigned int Npts)
  {
    // Initialisation
    double dx, x, y, x0, y0, x2, y2;
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // ntraj is the nuber of trajectories.
    // it is the number of iteration for a trajectory.
    long int ntraj = 0, it = 0;
    unsigned int ij[maxit*2], i;
    char diverge=0;
    double q;

    dx = (x_b[1]-x_b[0]) / (nx-1);
    while ( ntraj < maxtraj )
      {
        x0 = M_border[(2*it)%Npts]+gaussrand(dx);
        y0 = M_border[(2*it+1)%Npts]+gaussrand(dx);

        it = 0;
        x = x0;
        y = y0;
        x2 = x*x;
        y2 = y*y;
        diverge = 0;

        // Here we test if the point is obviously in the set
        // i.e. main cardioid and disk
        /* q = (x-1/4)*(x-1/4)+y2; */
        /* if (q*(q+(x-1/4))<y2/5 || (x+1)*(x+1)+y2<1/17){ */
        /*   // If the point is in the set we do not iterate */
        /*   // and we search for another */
        /*   it = maxit; */
        /* } */
        ij[it*2] = (y-y_b[0])/dx;
        ij[it*2+1] = (x-x_b[0])/dx;


        while ( it < maxit && diverge==0)
          {
            // Storing the trajectories
            y = 2 * x * y + y0;
            x = x2 - y2 + x0;
            x2 = x*x;
            y2 = y*y;
            it++;
            if (x_b[0] <= x && x <= x_b[1] && y_b[0] <= y && y <= y_b[1])
              {
                ij[it*2] = (y-y_b[0])/dx;
                ij[it*2+1] = (x-x_b[0])/dx;
              }
            if (x2+y2>16){
              diverge = 1;
            }
          }
        if (diverge == 1 && it>=minit)
          {
            ntraj++;
            for ( i=0; i<it-1; i+=2 )
              {
                *(M_traj+nx*ij[i]+ij[i+1]) += 1;
              }
            if (ntraj % 10000 == 0 && rank == 0)
              {
                printf("\e[?25l");
                printf("\rPoints computed by core 0 (x 1000) : %-6ld/%6ld   ", ntraj/1000, maxtraj/1000 );
                fflush(stdout);
              }
          }
      }
  }

unsigned int *zeros(unsigned int l, unsigned int m)
  {
    unsigned int *arr;
    arr = malloc(l*m*sizeof(int));
    if (arr==NULL){
        printf("\n ERROR No space allocated for the trajectories array\n");
    }
    unsigned int i, j;

    for (i=0; i<l; i++)
      {
        for (j=0; j<m; j++)
          {
            *(arr + i*m + j) = 0;
          }
      }
    return arr;
  }
