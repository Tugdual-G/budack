#!/usr/bin/env bash

# This script apply gamma transform to enhance the images
# Usefull only for big images with fewer point density


# Location of the executable, you shouldn't change this.
exegamma=core/gamma

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
inputdir=output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
outputdir=output/myfirstbudack/

# Parameters file
params_f=${outputdir}"param.txt"
# retrieving ny and nx from parameters file
nx=$(grep "nx" "${params_f}" | awk -F "=" '{print $2}')
ny=$(grep "ny" "${params_f}" | awk -F "=" '{print $2}')

printf "images output directory : \n"
printf "%s \n" "${outputdir}"

printf "Applying gamma transform \n"

if [[ -f "${outputdir}"traj0gamma.char ]]
   then
    rm "${outputdir}"traj0gamma.char
    rm "${outputdir}"traj1gamma.char
    rm "${outputdir}"traj2gamma.char
fi

mpiexec -n 3 "${exegamma}" ${nx} ${ny} 2 "${inputdir}" "${outputdir}" 1
echo "$nx" , "$ny"

magick -size "$nx"x"$ny" -depth 8\
    gray:"${outputdir}"traj0gamma.char gray:"${outputdir}"traj1gamma.char gray:"${outputdir}"traj2gamma.char \
    -channel RGB -combine -sigmoidal-contrast 5x50% -rotate 90 "${outputdir}"rgb0gamma.png

# magick -size "$nx"x"$ny" -depth 8\
#     gray:"${outputdir}"traj0gamma.char gray:"${outputdir}"traj1gamma.char gray:"${outputdir}"traj2gamma.char \
#     -channel RGB -combine -rotate 90 "${outputdir}"rgb0gamma.png

nsxiv "${outputdir}"rgb0*.png
