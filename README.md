# budack
 **Parallel colored rendering of the Mandelbrot set gost-points.**

([click to explore the details](https://raw.githubusercontent.com/Tugdual-G/budack/main/images_exemples/trajhd.png))

![alt text](images_exemples/zoom1.png)

## Usage
Just try the default parameters by running (see requirements),

    ./exe.sh

Different combinations of colors will be generated.
It takes less than half a second to compute the points on an 3GHz 4 core intel i5. 

The easiest way to run the computation is by using the script *exe.sh* which will also generate images.
This script use ImageMagick to create PNG images and sxiv to open them.
Editing the script allow to tune the parameters easily.

### Parameters
- The program use an uncommon parameter which greatly change the appearance of the generated images by fixing different depth zones for the starting points.
- The density of points per pixels control the noise of the images. 

You will also find parameters such as the size of the image, the maximum and minimum number of iterations.

The script exe.sh add more details to the function of each parameter in comments.


## Requirements
- Open-MPI for parallel computing
- ImageMagick, to create png pictures from the binary files.
 
            mpiexec --oversubscribe -n 3 budack [args]
    
- **Optional**
    - sxiv image viewer

**Note**
 If you only have one or two available cores, you may have to run the computation using the oversubscribe option in mpiexec such as (the script *exe.sh* take care of this case).
     
## Computing scheme

- Generate random points close to the border of the Mandelbrot set.
    - These points are computed randomly with a normal distribution around the border of the Mandelbrot set and stored on disk for reuse.
    The positions of the first points attract the probability distribution of the next points toward them. 
- Slightly offset these starting points randomly by a binomial distribution.
- Compute the trajectories until sufficient density (points per pixel) is reached.
- The trajectories are written to disk as 8 bits grayscale binaries and 16 bits grayscale binaries without any header (i.e. 2 files are written to disk for each color channel : red, green and blue). The parameters are written to disk too as 'param.txt' . 

**Note**   
Because of the lazy memory allocation by Linux kernel, the memory usage will start very low and grow as new pixels are visited by the trajectories. To avoid any surprise, the total allocated size for the arrays is shown at the beginning of the program, in practice the ram usage will be lower than the estimation.

## Images processing
- Use ImageMagick to generate images from 8 bites grayscale.
- Use a gamma function on the 16 bits grayscale to enhance smoothly the trajectories, which are then turned to 8 bits grayscale to generate images from 3x8 bites rgb channels. See the script *gamma.sh* This script is mostly needed for high resolution images. 
- The gamma function can also help distinguish the contribution of the different RGB channels.

**Snapshots:**
![alt text](images_exemples/colors.png)


![alt text](images_exemples/zoom.png)
