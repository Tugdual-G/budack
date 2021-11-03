#!/usr/bin/env bash

# Location of the executable, you shouldn't change this.
budack=core/budack
cwd=$(pwd)

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
trajdir0=${cwd}/output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
trajdir=${cwd}/output/UHD10_000_m300/

nx=10000 # Number of pixels in vertical direction
density=64 # Number of point per pixels, higher = less noise but slower
maxit=400 # Maximum number of iterations
minit=40 # Minimum number of iterations

# This will change greatly the apparence of the images,
# the closer it is to maxit, the faster the computation,
# but finer details will appear if it is low.
# This parameter will change the points where the normal distribution is centered.
# This coresspond to the max iteration (escape time) of these points.
depth=80
#depth=$(((maxit+minit)/2))

echo images output directory "${trajdir}"

# Image sizes (nx x ny):
if [ $nx = 1000 ];then ny=832;fi
if [ $nx = 2000 ];then ny=1666;fi
if [ $nx = 4000 ];then ny=3332;fi
if [ $nx = 8000 ];then ny=6666;fi
if [ $nx = 10000 ];then ny=8332;fi

# #------------- computing  -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx $maxit $minit $density $depth
mkdir "${trajdir}"
rm "${trajdir}"*
cp "${trajdir0}"* "$trajdir"

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

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb0.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj2.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb2.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb3.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb4.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj2.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb5.png

# echo --------- Opening image --------
# sxiv "${trajdir}"rgb0.png "${trajdir}"rgb1.png
# "${trajdir}"rgb2.png "${trajdir}"rgb3.png "${trajdir}"rgb4.png "${trajdir}"rgb5.png
#  "${trajdir}"hints.png "${trajdir}"gray.png
