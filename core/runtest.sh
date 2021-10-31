#!/usr/bin/env bash
budack=core/budack
trajdir0=output/traj0/
trajdir=output/trajtest
nx=1000
density=4

# Image sizes (nx x ny):
if [ $nx = 1000 ];then ny=832;fi
if [ $nx = 2000 ];then ny=1666;fi
if [ $nx = 4000 ];then ny=3332;fi
if [ $nx = 8000 ];then ny=6666;fi
#------------- computing  -------------
cd ..
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 200 0 $density
rm -r $trajdir
mv $trajdir0 $trajdir

echo --------- Creating gray scale images --------
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}/traj2.char \
      -rotate 90 ${trajdir}/gray.png

magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}/hints.char \
      -rotate 90 ${trajdir}/hints.png

echo --------- Creating colored image --------
magick convert -size ${nx}x${ny} -depth 8 \
    gray:${trajdir}/traj0.char gray:${trajdir}/traj1.char gray:${trajdir}/traj2.char \
    -channel GBR -combine -gamma 1 -rotate 90 ${trajdir}/rgb.png

echo --------- Opening image --------
sxiv ${trajdir}/hints.png ${trajdir}/gray.png ${trajdir}/rgb.png
