# budac
Generate beautifuls figures based on the Mandelbrot fractal via parallel cumputing.
![alt text](trajhd.png)
The trajectories are written on disk in 8 bits grayscale: 'trajectories.char'
Run with 'mpiexec ./budack' and display with python.
Generate image using imagemagick e.g for a grid of 1000x833: 
        ' magick convert -size 1000x833 -depth 8 GRAY:trajectories.char traj.png '

Generate random points close to the border of the Mandelbrot set; then compute their trajectories.
