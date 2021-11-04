#!/usr/bin/env bash
#
# This is the main executable.
# This script generate the trajectories and the images.

# Location of the executable, you shouldn't change this.
scrpt_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
budack="${scrpt_dir}"/core/budack

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
trajdir0="${scrpt_dir}"/output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
trajdir="${scrpt_dir}"/output/myfirstbudack/

nx=1000 # Number of pixels in vertical direction
density=16 # Number of point per pixels, higher = less noise but slower
maxit=200 # Maximum number of iterations
minit=40 # Minimum number of iterations

# Parameters file
params_f="${trajdir}"param.txt

# This will change greatly the apparence of the images,
# the closer it is to maxit, the faster the computation,
# but finer details will appear if it is low.
# This parameter will change the points where the normal distribution is centered.
# This coresspond to the max iteration (escape time) of these points.
depth=80
#depth=$(((maxit+minit)/2))

echo images output directory "${trajdir}"

# #------------- computing  -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 "$budack" "$nx" "$maxit" "$minit" "$density" "$depth"
mkdir "${trajdir}"
rm "${trajdir}"*
cp "${trajdir0}"* "$trajdir"

# Retrieving number of pixels in y direction
ny=$(grep 'ny' "${params_f}" | awk -F "=" '{print $2}')

echo --------- Creating gray scale images --------
# A grayscale image of the higher escape time points.
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:"${trajdir}"traj2.char \
      -rotate 90 "${trajdir}"gray.png

# Here we return an image of the points from where we randomly search
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:"${trajdir}"hints.char \
      -rotate 90 "${trajdir}"hints.png

echo --------- Creating colored image --------

# Here we create every possible RGB combinations

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char \
    -channel RGB -combine -noise 1 -rotate 90 "${trajdir}"rgb0.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj0.char gray:"${trajdir}"traj2.char gray:"${trajdir}"traj1.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb1.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj2.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb2.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb3.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb4.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb5.png

echo --------- Opening image --------
sxiv "${trajdir}"rgb[0-5].png "${trajdir}"hints.png "${trajdir}"gray.png
