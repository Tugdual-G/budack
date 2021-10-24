# budack
Generate beautiful figures based on the Mandelbrot fractal gost-points via parallel computing. 

([click to explore the rich details of the picture below](https://raw.githubusercontent.com/Tugdual-G/budack/main/trajectories_data/trajhd.png))

![alt text](trajectories_data/trajhd.png)

Run with mpiexec to compute and save the trajectories.

    mpiexec ./budack

The trajectories are written on disk as 8 bits per pixels binaries: 'trajectories.char' .
You can generate the image using imagemagick for example, for a grid of 1000x833:

    magick convert -size 1000x833 -depth 8 GRAY:trajectories.char traj.png

Using imagemagick's command-line utilities, you can also combine differents trajectories. For exemple, R.char , G.char , and B.char:

    magick convert -size 1000x833 -depth 8 R:R.char G:G.char B:B.char -combine -gamma 1.2 -rotate 90 colors.png 

Which gives:
![alt text](trajectories_data/colors.png)
or,
![alt text](trajectories_data/colors1.png)

Computing scheme :

- Generate random points close to the border of the Mandelbrot set.
- Offset these points randomly by a binomial distribution.
- Compute their trajectories.
