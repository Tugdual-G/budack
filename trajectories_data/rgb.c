#include <stdio.h>
#include <stdlib.h>

int main()
  {
    // Assuming the data is normalized to the range 0:255;
    unsigned int nx = 1000, ny=833;
    unsigned int size = ny*nx, i, j;
    unsigned char *R = (unsigned char *)malloc(size*sizeof(unsigned char));
    unsigned char *G = (unsigned char *)malloc(size*sizeof(unsigned char));
    unsigned char *B = (unsigned char *)malloc(size*sizeof(unsigned char));
    unsigned char *RGB = (unsigned char *)malloc(3*size*sizeof(unsigned char));

    FILE *fp;
    fp = fopen("R.char", "rb");
    fread(R, sizeof(unsigned char), size, fp);
    fclose(fp);
    fp = fopen("G.char", "rb");
    fread(G, sizeof(unsigned char), size, fp);
    fclose(fp);
    fp = fopen("B.char", "rb");
    fread(B, sizeof(unsigned char), size, fp);
    fclose(fp);

    /* for (i=0; i<ny; i+=3) */
    /*   { */
    /*     for (j=0; j<nx; j++) */
    /*       { */
    /*         RGB[nx*i+j] = R[nx*i+j]; */
    /*         RGB[nx*(1+i)+j] = G[nx*i+j]; */
    /*         RGB[nx*(2+i)+j] = B[nx*i+j]; */
    /*       } */
    /*   } */
    for (i=0; i<size*3-2; i+=3)
      {

        RGB[i]=R[i];
        RGB[i+1]=G[i];
        RGB[i+2]=B[i];
      }

    free(R);
    free(G);
    free(B);

    fp = fopen("img", "wb");
    if(fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fwrite(RGB, sizeof(unsigned char), 3*size, fp);
    printf(" depth %lu \n",sizeof(unsigned char));

    fclose(fp);
    free(RGB);

    return 0;
  }
