# budack
 **Parallel computation of the Mandelbrot set gost-points.**

([click to explore the rich details](https://raw.githubusercontent.com/Tugdual-G/budack/main/images_exemples/trajhd.png))

![alt text](images_exemples/zoom1.png)

The easiest way to run the computation is by using the script exe.sh which will also generate images. This script use ImageMagick to create PNG images and sxiv to open them.

    ./exe.sh

Otherwise, if you just want to compute and save the trajectories.

    mpiexec ./budack

The trajectories are written on disk as 8 bits per pixels binaries: 'trajectories.char'.
You can generate the images using ImageMagick, for example, for a grid of 1000x833:

    magick convert -size 1000x833 -depth 8 GRAY:trajectories.char traj.png

Using ImageMagick's command-line utilities, you can also combine different trajectories. For example, R.char , G.char , and B.char:

    magick convert -size 1000x833 -depth 8 R:R.char G:G.char B:B.char -combine -gamma 1.2 -rotate 90 colors.png 
    
**Snapshots:**
![alt text](images_exemples/colors.png)



![alt text](images_exemples/zoom.png)

**Computing scheme** :

- Generate random points close to the border of the Mandelbrot set.
    - These points are computed randomly with a normal distribution around the border of the Mandelbrot set. The values of the first N points attract the probability distribution of the next point toward them. 
- Slightly offset these points by a binomial distribution.
- Compute their trajectories.
- The trajectories are written to disk as 8 bits grayscale binaries and 16 bits grayscale binaries without any header (i.e. 2 files are written to disk for each color channel : red, green and blue). The N first points and the parameters are written to disk too. 

**Images processing:**
- Use ImageMagick to generate images from 8 bites grayscale.
- Use a gamma function on the 16 bits grayscale to enhance smoothly the trajectories, which are then turned to 8 bits grayscale to generate images from 3x8 bites rgb channels.
