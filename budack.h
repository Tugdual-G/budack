#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double randomfloat(double min, double max)
  {
    double r, range=max-min;
    r = (double)rand()/(double)(RAND_MAX);
    r = min + r * range;
    return r;
  }

void trajectories(
    int nx, int ny, double x_b[2],
    double y_b[2], int M_traj[ny][nx],
    long int maxtraj, int maxit)
  {
    // Initialisation
    double dx, x, y, x0, y0, x2, y2;
    // ntraj is the nuber of trajectories.
    // it is the number of iteration for a trajectory.
    long int ntraj = 0, it = 0, max_acumul=0;
    int ij[maxit*2], i;
    char diverge=0;

    dx = (x_b[1]-x_b[0]) / (nx-1);
    printf("dx = %f \n", dx);
    while ( ntraj < maxtraj )
      {
        x0 = randomfloat(x_b[0],x_b[1]);
        y0 = randomfloat(0, y_b[1]);
        it = 0;
        x = x0;
        y = y0;
        x2 = x*x;
        y2 = y*y;
        diverge = 0;
        while ( it < maxit && diverge==0)
          {
            // Storing the trajectories
            ij[it*2] = (y-y_b[0])/dx;
            ij[it*2+1] = (x-x_b[0])/dx;
            y = 2 * x * y + y0;
            x = x2 - y2 + x0;
            x2 = x*x;
            y2 = y*y;
            if (x_b[0] >= x || x >= x_b[1] || y_b[0] >= y || y >= y_b[1])
              {
                diverge = 1;
              }
            it++;
          }
        if (diverge == 1)
          {
            ntraj++;
            for ( i=0; i<it-1; i+=2 )
              {
                M_traj[ij[i]][ij[i+1]] += 1;
                if (M_traj[ij[i]][ij[i+1]]>max_acumul)
                  {
                    max_acumul = M_traj[ij[i]][ij[i+1]] ;
                  }
              }
          }
      }
    printf("Max acumulated : %d \n", max_acumul );
  }
