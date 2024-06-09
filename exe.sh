#!/usr/bin/env bash

# This is the main executable.
# This script generate the trajectories and the images.

# Location of the executable, you shouldn't change this.
budack=core/budack

if [[ ! -f "${budack}" ]]
    then
    make
fi
# The computation need 3 cores to run properly
# if les than 3 cores available, we use the oversubscribe option.
ncores=$(getconf _NPROCESSORS_ONLN)
oversub=
if [ $ncores -lt 3 ]
   then
    printf "Less than 3 cores available\n"
    oversub="--oversubscribe -n 3"
fi

# Output of the computation
trajdir0=/tmp/budack/
if [[ ! -d "${trajdir0}" ]]
   then
   mkdir "${trajdir0}"
fi

# Output directory of the enhanced images
trajdir=output/
if [[ ! -d "${trajdir}" ]]
   then
    mkdir "${trajdir}"
fi

nx=800 # Number of pixels in vertical direction
density=10 # Number of point per pixels, higher = less noise but slower
maxit=600 # Maximum number of iterations
minit=60 # Minimum number of iterations

# maxit=1000 # Maximum number of iterations
# minit=50 # Minimum number of iterations

# Parameters file
params_f="${trajdir}"param.txt

# This will change greatly the apparence of the images,
# the closer it is to maxit, the faster the computation,
# but finer details will appear if it is low.
# This parameter will change the points where the normal distribution is centered.
# This coresspond to the max iteration (escape time) of these points.
depth=60
#depth=$(((maxit+minit)/2))

# #------------- computing  -------------
mpiexec $oversub "$budack" "$nx" "$maxit" "$minit" "$density" "$depth" "$trajdir0" &

# Since open-MPI last version this is necesary in order to know the progress of the computation.
START_TIME=$SECONDS
echo 0 > /tmp/progress
value="0"
while [ "$value" == "0" ]
do
    value=$(cat /tmp/progress)
done
while [ "$value" != "0" ]
do
    ELAPSED_TIME=$(($SECONDS - $START_TIME))
    printf "\r%s  %u s" "$value" "$ELAPSED_TIME"
    value=$(cat /tmp/progress)
    sleep 0.5
done

contrast="-sigmoidal-contrast 20x0%"
magick "${trajdir0}"image.tiff ${contrast} -rotate 90 "${trajdir}"image.png
echo "written enhanced image (sigmoidal contrast) ," "${trajdir}"image.png
nsxiv "${trajdir}"image.png
wait
