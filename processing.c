#include <stdio.h>
#include <stdlib.h>

int main()
  {
    FILE *fptr;
    unsigned int *B_sum;
    unsigned long size;
    fptr = fopen("trajectories.data", "r+");
    if (fptr==NULL){printf("The file trajectories.data does not exist."); return 0;}
    size = fread(B_sum, sizeof(unsigned int)*1000*833, 1 , fptr);
    printf("size %lu \n", sizeof(fptr));
    B_sum = (unsigned int *) malloc(size);


    long k, kmax=1000;
    unsigned int bmax=0;

    for (k=0; k<size; k++)
        {
            if (*(B_sum+k)>bmax)
                {
                    bmax = *(B_sum+k);
                    printf("%u \n", bmax );
                }
        }
    /* for (k=0; k<size; k++) */
    /*     { */
    /*         *(B_sum+k) = *(B_sum+k)/(16) ; */
    /*     } */

    /* fwrite(B_sum, sizeof(unsigned int)*1000*833, 1, fptr); */
    return 0;
  }
