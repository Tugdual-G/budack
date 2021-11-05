# budack
 **Parallel computation of the Mandelbrot set gost-points.**

([click to explore the rich details](https://raw.githubusercontent.com/Tugdual-G/budack/main/images_exemples/trajhd.png))

![alt text](images_exemples/zoom1.png)

**Usage**

The easiest way to run the computation is by using the script exe.sh which will also generate images. This script use ImageMagick to create PNG images and sxiv to open them. Editing the script allow to tune the most relevant parameters easily.

    ./exe.sh

Different combinations of colors will be generated.

You can compile the code by running make.

    cd core/
    make
    
**Computing scheme**

- Generate random points close to the border of the Mandelbrot set.
    - These points are computed randomly with a normal distribution around the border of the Mandelbrot set and stored on disk for reuse. The positions of the first points attract the probability distribution of the next points toward them. 
- Slightly offset these starting points randomly by a binomial distribution.
- Compute the trajectories until sufficient density (points per pixel) is reached.
- The trajectories are written to disk as 8 bits grayscale binaries and 16 bits grayscale binaries without any header (i.e. 2 files are written to disk for each color channel : red, green and blue). The parameters are written to disk too as 'param.txt' . 

**Images processing**
- Use ImageMagick to generate images from 8 bites grayscale.
- Use a gamma function on the 16 bits grayscale to enhance smoothly the trajectories, which are then turned to 8 bits grayscale to generate images from 3x8 bites rgb channels. This script is mostly needed for high resolution images. 
   
**Snapshots:**
![alt text](images_exemples/colors.png)


![alt text](images_exemples/zoom.png)
