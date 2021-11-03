#!/usr/bin/env bash
scrpt_dir="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "${scrpt_dir}"
cd ..
cwd=$(pwd)
budack=core/budack
trajdir0=output/traj0/
trajdir=output/test1/
nx=4000
density=16
maxit=200
minit=40
#depth=$(((maxit+minit)/2))
depth=80

echo images output directory "${cwd}"/"${trajdir}"

# Image sizes (nx x ny):
if [ $nx = 1000 ];then ny=832;fi
if [ $nx = 2000 ];then ny=1666;fi
if [ $nx = 4000 ];then ny=3332;fi
if [ $nx = 8000 ];then ny=6666;fi

# #------------- computing  -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx $maxit $minit $density $depth
mkdir ${trajdir}
rm ${trajdir}*
mv ${trajdir0}* $trajdir

echo --------- Creating gray scale images --------
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}traj2.char \
      -rotate 90 ${trajdir}gray.png

magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}hints.char \
      -rotate 90 ${trajdir}hints.png

echo --------- Creating colored image --------
magick convert -size ${nx}x${ny} -depth 8 \
    gray:${trajdir}traj0.char gray:${trajdir}traj1.char gray:${trajdir}traj2.char \
    -channel RGB -combine -rotate 90 ${trajdir}rgb0.png

echo --------- Opening image --------
sxiv ${trajdir}rgb0.png ${trajdir}hints.png ${trajdir}gray.png
