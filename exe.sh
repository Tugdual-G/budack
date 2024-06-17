#!/usr/bin/env bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./core
# This is the main executable.
# This script generate the trajectories and the images.

nx=1000 # Number of pixels in vertical direction
density=20 # Number of point per pixels, higher = less noise but slower
maxit=500 # Maximum number of iterations
minit=60 # Minimum number of iterations

#for live rendering, set to 0 to disable live rendering
cycles_per_frame_update=2

# PNG_IMAGE_NAME=nx"${nx}"_minit"${minit}"_maxit"${maxit}".png
PNG_IMAGE_NAME=image.png

# Output of the computation
RAW_OUTPUT_DIR=/tmp/budack/

# Output directory of the enhanced images
ENHANCED_OUTPUT_DIR=/tmp/budack/


# This will change greatly the apparence of the images,
# the closer it is to maxit, the faster the computation,
# but finer details will appear if it is low.
# This parameter will change the points where the normal distribution is centered.
# This coresspond to the max iteration (escape time) of these points.
depth=${minit}

# Location of the executable, you shouldn't change this.
budack=core/budack

if [[ ! -f "${budack}" ]]
    then
    make -C core
fi

# The computation need 3 cores to run properly
# if les than 3 cores available, we use the oversubscribe option.
ncores=$(getconf _NPROCESSORS_ONLN)
oversub=
if [ $ncores -lt 3 ]
   then
    printf "Less than 3 cores available\n"
    oversub="--oversubscribe -n 4"
elif [ $ncores -gt 5 ]
    then
    oversub="-n 5"
fi

# Output of the computation
if [[ ! -d "${RAW_OUTPUT_DIR}" ]]
   then
   mkdir "${RAW_OUTPUT_DIR}"
fi

# Output directory of the enhanced images
if [[ ! -d "${ENHANCED_OUTPUT_DIR}" ]]
   then
    mkdir "${ENHANCED_OUTPUT_DIR}"
fi

# #------------- computing  -------------
mpiexec $oversub "$budack" "$nx" "$maxit" "$minit" "$density" "$depth" "$RAW_OUTPUT_DIR" "$cycles_per_frame_update"

# Since open-MPI last version this is necesary in order to know the progress of the computation.
# START_TIME=$SECONDS
# echo 0 > /tmp/progress
# value="0"

# while [ "$value" == "0" ]
# do
#     value=$(</tmp/progress)
# done

# printf "\e[?25l"

# while [ "$value" != "0" ]
# do
#     ELAPSED_TIME=$((SECONDS - START_TIME))
#     printf "\r%s  %u s" "$value" "$ELAPSED_TIME"
#     value=$(</tmp/progress)
#     sleep 0.1
# done
# printf "\e[?25h"
# wait


# contrast="-sigmoidal-contrast 10x10%"
# magick "${RAW_OUTPUT_DIR}"image.tiff ${contrast} -rotate 90 "${ENHANCED_OUTPUT_DIR}""${PNG_IMAGE_NAME}"
# echo "written enhanced image (sigmoidal contrast) ," "${ENHANCED_OUTPUT_DIR}""${PNG_IMAGE_NAME}"
# if [[ $cycles_per_frame_update -eq 0 ]]
#    then
#     xdg-open "${ENHANCED_OUTPUT_DIR}""${PNG_IMAGE_NAME}"
# fi
