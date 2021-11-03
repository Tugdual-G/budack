#!/usr/bin/env bash

# This script aplly gamma transform to enhance the images

# Location of the executable, you shouldn't change this.
exegamma=core/gamma
cwd=$(pwd)

# Output of the computation, it is hardcoded in the c program for now
# you shouldn't change this.
trajdir0=${cwd}/output/traj0/

# Output directory of the images, all the data will be moved
# here (param.txt , traj.char , etc...),
# you can change the name of the dir if you want:
trajdir=${cwd}/output/UHD10_000_m300/

# nx=$(grep "nx" "${trajdir}"param.txt | awk -F "=" '{print $2}')
 nx=10000 # Number of pixels in vertical direction

echo images output directory "${trajdir}"

# Image sizes (nx x ny):
if [ $nx = 1000 ];then ny=832;fi
if [ $nx = 2000 ];then ny=1666;fi
if [ $nx = 4000 ];then ny=3332;fi
if [ $nx = 8000 ];then ny=6666;fi
if [ $nx = 10000 ];then ny=8332;fi
if [ $nx = 16000 ];then ny=13332;fi

echo --------- Applying gamma transform --------
cd "${trajdir}" || (echo cannot change ; $(exit))
rm "${trajdir}"traj0gamma.char
rm "${trajdir}"traj1gamma.char
rm "${trajdir}"traj2gamma.char
mpiexec --mca opal_warn_on_missing_libcuda 0 -n 3 ~/Documents/Programmation/C/budac/core/gamma ${nx} ${ny} 1.8

echo --------- Creating colored image --------
magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj2gamma.char \
    -channel RGB -combine -noise 2 -rotate 90 "${trajdir}"rgb0gamma.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj1gamma.char \
    -channel RGB -combine -enhance -rotate 90 "${trajdir}"rgb1gamma.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj2gamma.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb2gamma.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj0gamma.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb3gamma.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj0gamma.char gray:"${trajdir}"traj1gamma.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb4gamma.png

magick convert -size ${nx}x${ny} -depth 8 \
    gray:"${trajdir}"traj2gamma.char gray:"${trajdir}"traj1gamma.char gray:"${trajdir}"traj0gamma.char \
    -channel RGB -combine -rotate 90 "${trajdir}"rgb5gamma.png

echo --------- Opening image --------
sxiv "${trajdir}"rgb0gamma.png "${trajdir}"rgb0gamma.png \
 "${trajdir}"rgb2gamma.png "${trajdir}"rgb3gamma.png "${trajdir}"rgb4gamma.png "${trajdir}"rgb5gamma.png
