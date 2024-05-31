#!/usr/bin/env bash

# This is the main executable.
# This script generate the trajectories and the images.

# Location of the executable, you shouldn't change this.
budack=core/budack

# The computation need 3 cores to run properly
# if les than 3 cores available, we use the oversubscribe option.
ncores=$(getconf _NPROCESSORS_ONLN)
oversub=
if [ $ncores -lt 3 ]
   then
    printf "Less than 3 cores available\n"
    oversub="--oversubscribe -n 3"
fi

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
trajdir0=output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the directory if you want:
trajdir=output/myfirstbudack/

nx=1000 # Number of pixels in vertical direction
density=10 # Number of point per pixels, higher = less noise but slower
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

# #------------- computing  -------------
mpiexec $oversub "$budack" "$nx" "$maxit" "$minit" "$density" "$depth"


if [[ ! -d "${trajdir}" ]]
   then
    mkdir "${trajdir}"
else
    rm "${trajdir}"*
fi

cp "${trajdir0}"* "$trajdir"

# Retrieving number of pixels in y direction
ny=$(grep 'ny' "${params_f}" | awk -F "=" '{print $2}')

printf "\nimages output directory : \n"
printf "%s \n" "${trajdir}"

printf "Creating gray scale images"

# A grayscale image of the higher escape time points.
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:"${trajdir}"traj2.char \
      -sigmoidal-contrast 10x0% -rotate 90 "${trajdir}"gray.png

# Here we return an image of the points from where we randomly search
# magick convert -size ${nx}x${ny} -depth 8 \
#      GRAY:"${trajdir}"hints.char \
#       -sigmoidal-contrast 10x0% -rotate 90 "${trajdir}"hints.png

printf "\x1b[2K \rCreating colored images \n"
# Here we create every possible RGB combinations
#
set -o xtrace

magick -size ${nx}x${ny} -depth 8\
    gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb0.png

magick -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj0.char gray:"${trajdir}"traj2.char gray:"${trajdir}"traj1.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb1.png

magick -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj2.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb2.png

magick -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1.char gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb3.png

magick -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2.char gray:"${trajdir}"traj0.char gray:"${trajdir}"traj1.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb4.png

magick -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2.char gray:"${trajdir}"traj1.char gray:"${trajdir}"traj0.char \
    -channel RGB -combine -sigmoidal-contrast 10x5% -rotate 90 "${trajdir}"rgb5.png

printf "\x1b[2K \rOpening images"
#xdg-open "${trajdir}"rgb[0-5].png "${trajdir}"hints.png "${trajdir}"gray.png
xdg-open "${trajdir}"rgb0.png
set +o xtrace
printf "\x1b[2K \n"
