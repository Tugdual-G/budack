# budack
 Parallel computation of the Mandelbrot set gost-points. 

([click to explore the rich details](https://raw.githubusercontent.com/Tugdual-G/budack/main/images_exemples/trajhd.png))

![alt text](images_exemples/zoom1.png)

Run with mpiexec to compute and save the trajectories.

    mpiexec ./budack

The trajectories are written on disk as 8 bits per pixels binaries: 'trajectories.char'.
You can generate the image using ImageMagick for example, for a grid of 1000x833:

    magick convert -size 1000x833 -depth 8 GRAY:trajectories.char traj.png

Using ImageMagick's command-line utilities, you can also combine different trajectories. For example, R.char , G.char , and B.char:

    magick convert -size 1000x833 -depth 8 R:R.char G:G.char B:B.char -combine -gamma 1.2 -rotate 90 colors.png 

Which gives:
![alt text](images_exemples/colors.png)

or,

![alt text](images_exemples/zoom.png)

Computing scheme :

- Generate random points close to the border of the Mandelbrot set.
    - These points are computed randomly with a normal distribution around the border of the mandelbrot set. The values of the first N points attract the probability distribution of the next point toward them. When N is small (N=100) this first part of computation is faster and the generated pictures presents some interesting random nuances.  
- Slightly offset these points by a binomial distribution.
- Compute their trajectories.
- The trajectories are writen on disk as 8 bits grayscale binaries and 16 bits grayscale binaries whithout any header (i.e. 2 files are written to disk for red, green and blue). The N fisrt points are written to disk too.

Images generation:
- Use ImageMagick to generate images from 8 bites grayscale.
- Use a gamma function on the 16 bits grayscale to enhance smoosly which are then turned to 8 bits grayscale to generate images from 3x8 bites rgb channels.
