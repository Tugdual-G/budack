# budack
Generate beautiful figures based on the Mandelbrot fractal via parallel computing. 

(click on the picture to see the rich details)
![alt text](trajhd.png)
The trajectories are written on disk in 8 bits grayscale: 'trajectories.char' . This format can be easily changed to higher quality.

Run with mpiexec to compute and save the trajectories.

    mpiexec ./budack

You can generate the image using imagemagick for example, for a grid of 1000x833:

    magick convert -size 1000x833 -depth 8 GRAY:trajectories.char traj.png

Computing scheme :

Generate random points close to the border of the Mandelbrot set, then compute their trajectories.
