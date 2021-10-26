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
    double *M_brdr,
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
    long int mean_it_per_point=0;

    dx = (x_b[1]-x_b[0]) / nx;
    while ( ntraj < maxtraj )
      {
        y0 = M_brdr[(2*itraj)%Nborder]+gaussrand(5*dx);
        x0 = M_brdr[(2*itraj+1)%Nborder]+gaussrand(5*dx);

        it = 0;
        x = x0;
        y = y0;
        x2 = x*x;
        y2 = y*y;
        diverge = 0;

        /* // Here we test if the point is obviously in the set */
        /* // i.e. main cardioid and disk */
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
            mean_it_per_point += it;
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
    if (rank==0)
      {
        mean_it_per_point = mean_it_per_point/maxtraj;
        printf("\nMean it per starting point : %ld , for maxit = %u , minit = %u \n", mean_it_per_point, maxit, minit);
      }
  }


void border(float x_b[2],
            float y_b,
            unsigned int depth,
            long int Npts,
            unsigned int *M,
            double *M_brdr)
  {
    // Return the list of the points at the boundary in index coordinates
    // relative to the subdomain bodaries.
    double x, y, x0, y0, x2, y2;
    unsigned int it=0, start=500;
    unsigned int k=0, n=0;
    unsigned char mindepth = depth*0.9;
    float sigma=50/start;

    while (2*k<Npts)
      {
        it = 0;
        if (k<start)
          {
            x0 = randomfloat(-2, 1);
            y0 = randomfloat(-1.5, 1.5);
          }
        else
          {
            n = n%k;
            x0 = *(M_brdr+n*2+1)+gaussrand(sigma);
            y0 = *(M_brdr+n*2)+gaussrand(sigma);
            n+=20;
          }
        x = x0;
        y = y0;
        x2 = x*x;
        y2 = y*y;

        while ( it < depth && x2+y2 < 4)
          {
            // Storing the trajectories
            y = 2 * x * y + y0;
            x = x2 - y2 + x0;
            x2 = x*x;
            y2 = y*y;
            it++;
          }
        if (it < depth && it >= mindepth)
          {
            *(M_brdr+k*2)= y0;
            *(M_brdr+k*2+1) = x0;
            k++;
          }
      }
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
