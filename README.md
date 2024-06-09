# budack
 **Parallel colored rendering of the Mandelbrot set gost-points.**

([click to explore the details](https://raw.githubusercontent.com/Tugdual-G/budack/main/images_exemples/trajhd.png))

![alt text](images_exemples/zoom1.png)

## Usage
Just try the default parameters by running (see requirements),

    ./exe.sh

The main program create a 16 bits per channel TIFF image to favor further processing.
ImageMagick is used to apply a sigmoidal contrast to the image, but this can be done with any editing software.
Editing the script allow to tune the parameters easily.

### Parameters
- The program use an uncommon parameter which greatly change the appearance of the generated images by fixing different depth zones for the starting points.
- The density of points per pixels control the noise of the images. 

You will also find parameters such as the size of the image, the maximum and minimum number of iterations.

The script exe.sh add more details to the function of each parameter in comments.


## Requirements
- Open-MPI for parallel computing
- libtiff c library

**Optional**
- ImageMagick, to process and enhance the TIFF files.

**Note**
 If you only have one or two available cores, you may have to run the computation using the oversubscribe option in mpiexec such as (the script *exe.sh* take care of this case).
      
            mpiexec --oversubscribe -n 3 budack [args]
    
## Computing scheme

- Generate random points close to the border of the Mandelbrot set.
    - These points are computed randomly with a normal distribution around the border of the Mandelbrot set and stored on disk for reuse.
    The positions of the first points attract the probability distribution of the next points toward them. 
- Slightly offset these starting points randomly by a binomial distribution.
- Compute the trajectories until sufficient density (points per pixel) is reached.
- An output .tiff image is created. The parameters are written to disk too as 'param.txt'. 

**Note**   
Because of the lazy memory allocation by Linux kernel, the memory usage will start very low and grow as new pixels are visited by the trajectories. To avoid any surprise, the total allocated size for the arrays is shown at the beginning of the program, in practice the ram usage will be lower than the estimation.

## Images processing
- Use ImageMagick to process the images.
- Use a gamma function on the 16 bits grayscale to enhance smoothly the trajectories, which are then turned to 8 bits grayscale to generate images from 3x8 bites rgb channels. See the script *gamma.sh* This script is mostly needed for high resolution images. 
- The gamma function can also help distinguish the contribution of the different RGB channels.



