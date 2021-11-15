#!/usr/bin/env bash

# This script aplly gamma transform to enhance the images


# Location of the executable, you shouldn't change this.
scrpt_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
exegamma="${scrpt_dir}"/core/gamma

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
trajdir0="${scrpt_dir}"/output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
trajdir="${scrpt_dir}"/output/myfirstbudack/

# Parameters file
params_f="${trajdir}"param.txt
# retrieving ny and nx from parameters file
nx=$(grep "nx" "${params_f}" | awk -F "=" '{print $2}')
ny=$(grep "ny" "${params_f}" | awk -F "=" '{print $2}')

printf "images output directory : \n"
printf "%s \n" "${trajdir}"

printf "Applying gamma transform"
cd "${trajdir}" || (echo cannot change ; $(exit))

if [[ -f "${trajdir}"traj0gamma.char ]]
   then
    rm "${trajdir}"traj0gamma.char
    rm "${trajdir}"traj1gamma.char
    rm "${trajdir}"traj2gamma.char
fi

mpiexec -q -n 3 "${exegamma}" ${nx} ${ny} 1.8

printf "\x1b[2K \rCreating colored images"

# magick convert -size "${nx}x${ny}" -depth 8 \
#     gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj2gamma.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb0gamma.png

magick convert -size "${nx}x${ny}" -depth 8 \
    gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj1gamma.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb1gamma.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj2gamma.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb2gamma.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj0gamma.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb3gamma.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj1gamma.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb4gamma.png

# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj0gamma.char \
#     -channel RGB -combine -rotate 90 "${trajdir}"rgb5gamma.png

printf "\x1b[2K \rOpening images"
# magick convert "${trajdir}"rgb0gamma.png -crop 1920x1080+3200+2200 "${trajdir}"croppedg0.jpg
sxiv "${trajdir}"*gamma.png
printf "\x1b[2K \n"
