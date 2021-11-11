#!/usr/bin/env bash
#
# This is the main executable.
# This script generate the trajectories and the images.

# Location of the executable, you shouldn't change this.
scrpt_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
budack="${scrpt_dir}"/core/budack

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
trajdir0="${scrpt_dir}"/output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
trajdir="${scrpt_dir}"/output/myfirstbudack/

nx=1000 # Number of pixels in vertical direction
density=8 # Number of point per pixels, higher = less noise but slower
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
mpiexec $oversub -q "$budack" "$nx" "$maxit" "$minit" "$density" "$depth"

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
      -rotate 90 "${trajdir}"gray.png

# Here we return an image of the points from where we randomly search
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:"${trajdir}"hints.char \
      -rotate 90 "${trajdir}"hints.png

printf "\x1b[2K \rCreating colored images"
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

printf "\x1b[2K \rOpening images"
sxiv "${trajdir}"rgb[0-5].png "${trajdir}"hints.png "${trajdir}"gray.png
printf "\x1b[2K \n"
