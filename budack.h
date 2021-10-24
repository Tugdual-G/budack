#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>


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

  return Z*dx;
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
    unsigned int *M_traj,
    long int maxtraj,
    int maxit,
    int minit,
    unsigned int *M_brdr,
    unsigned int Nborder)
  {
    // Initialisation
    int rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    double dx, x, y, x0, y0, x2, y2;

    // ntraj is the nuber of trajectories.
    // it is the number of iteration for a trajectory.
    long int ntraj = 0, it = 0, itraj=0;
    unsigned int ij[maxit*2+1], i;
    char diverge=0;
    double q;

    dx = (x_b[1]-x_b[0]) / nx;
    while ( ntraj < maxtraj )
      {
        y0 = (1/2+M_brdr[(2*itraj)%Nborder])*dx+gaussrand(5*dx);
        x0 = x_b[0]+(1/2+M_brdr[(2*itraj+1)%Nborder])*dx+gaussrand(5*dx);

        it = 0;
        x = x0;
        y = y0;
        x2 = x*x;
        y2 = y*y;
        diverge = 0;

        // Here we test if the point is obviously in the set
        // i.e. main cardioid and disk
        q = (x-1/4)*(x-1/4)+y2;
        if (q*(q+(x-1/4))<y2/5 || (x+1)*(x+1)+y2<1/17){
          // If the point is in the set we do not iterate
          // and we search for another
          it = maxit;
        }
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
            if (x_b[0] < x && x < x_b[1] && y_b[0] < y && y < y_b[1])
              {
                ij[it*2] = (y-y_b[0])/dx;
                ij[it*2+1] = (x-x_b[0])/dx;
              }
            if (x2+y2>16)
              {
                diverge = 1;
              }
          }
        if (diverge == 1 && it>minit)
          {
            ntraj++;
            for ( i=0; i<it; i+=2 )
              {
                *(M_traj+nx*ij[i]+ij[i+1]) += 1;
              }
            if (rank==0 && ntraj % 10000 == 0)
              {
                printf("\rPoints computed by core 0 (x 1000) : %-6ld/%6ld", ntraj/1000, maxtraj/1000 );
                fflush(stdout);
              }
          }
        itraj++;
      }
  }


void border(float x_b[],
            float y_b,
            unsigned char depth,
            unsigned int nx,
            unsigned int *Npts,
            unsigned int *M,
            unsigned int *M_brdr)
  {
    // Return the list of the points at the boundary in index coordinates
    // relative to the subdomain bodaries.
    double dx, x, y, x0, y0, x2, y2, q;
    unsigned int ny, it=0, i, j;
    unsigned int k=0;
    unsigned char mindepth = depth*0.4;
    dx = (x_b[1]-x_b[0]) / nx;
    ny = y_b/dx;

    for (i=0;i<ny; i++)
      {
        for (j=0; j<nx; j++)
          {
            it = 0;
            x = 0;
            y = 0;
            x2 = 0;
            y2 = 0;
            x0 = x_b[0]+(1/2+j)*dx;
            y0 = (1/2+i)*dx;

            /* Here we test if the point is obviously in the set */
            /* i.e. main cardioid and disk */
            q = (x-1/4)*(x-1/4)+y2;
            if (q*(q+(x-1/4))<y2/5 || (x+1)*(x+1)+y2<1/17){
              // If the point is in the set we do not iterate
              // and we search for another
              it = depth;
            }

            while ( it < depth && x2+y2 < 4)
              {
                // Storing the trajectories
                y = 2 * x * y + y0;
                x = x2 - y2 + x0;
                x2 = x*x;
                y2 = y*y;
                *(M+i*nx+j) += 1;
                it++;
              }
            if (it < depth && it >= mindepth && k*2<*Npts)
              {
                *(M_brdr+k*2)= i;
                *(M_brdr+k*2+1) = j;
                k++;
              }
          }
      }
    if (k*2>=*Npts)
      {
        printf("\nWarning : Not enough space allocated to store all the border points, \n");
        printf("set parameter Npts larger.\n");
      }
    *Npts = 2*k;
  }

void save(char fname[], void *data, unsigned int size)
  {
    FILE *fp;
    fp = fopen(fname, "w+");
    if(fp == NULL)
    {
        printf("Error opening file %s, cannot save.\n", fname);
        exit(1);
    }
    fwrite(data, size, 1, fp);
    fclose(fp);
  }

void mirror_traj(unsigned int ny, unsigned int nx, unsigned int *B)
  {
    unsigned long k;
    unsigned int i, j;
    unsigned int b;

    for (i=0;i<ny;i++)
        {
          for(j=0;j<nx;j++)
            {
              k = (unsigned long) ny*nx-(1+i)*nx+j;
              b = B[i*nx+j];
              B[i*nx+j] += B[k];
              B[k] += b;
            }

        }
  }

void save_chargrayscale(unsigned int ny, unsigned int nx, unsigned int *B, char fname[])
  {
    unsigned long size = nx*ny;
    unsigned char *B_c;
    B_c = (unsigned char *) malloc(sizeof(unsigned char)*size);

    unsigned long k;
    unsigned int bmax=0;

    mirror_traj(ny,nx,B);

    for (k=0; k<size; k++)
        {
            if (*(B+k)>bmax)
                {
                    bmax = *(B+k);
                }
        }
    printf("Maximum accumulted points %u \n", bmax );
    for (k=0; k<size; k++)
        {
            *(B_c+k) = (double) *(B+k) * 255/bmax ;
        }


    FILE *fptr;
    fptr = fopen(fname, "w+");
    fwrite(B_c, sizeof(unsigned char), size, fptr);
    fclose(fptr);
    free(B_c);
  }
