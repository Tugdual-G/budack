#!/usr/bin/env bash

dir=/home/tugdual/Documents/Programmation/C/budac/
budackc=${dir}budack.c
budack=${dir}budack
trajdir=${dir}trajectories_data/
traj=${trajdir}traj
imgname=temp
nx=4000
startingpts=50000
density=4

# Image sizes (nx x ny):
#
# 1000x832
# 2000x1666
# 4000x3332
# 8000x6666
if [ $nx = 1000 ];then ny=832;fi
if [ $nx = 2000 ];then ny=1666;fi
if [ $nx = 4000 ];then ny=3332;fi
if [ $nx = 8000 ];then ny=6666;fi
#------------- computing  -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 800 0 $startingpts $density
# mv ${traj}0.char ${trajdir}r.char
# mv ${traj}1.char ${trajdir}g.char
mv ${traj}2.char ${trajdir}b.char

echo --------- Creating gray scale images --------
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}b.char \
      -gamma 1.2 -rotate 90 ${trajdir}${imgname}gray.png

magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}hints.char \
      -rotate 90 ${trajdir}${imgname}hints.png

# echo --------- Creating colored image --------
# magick convert -size ${nx}x${ny} -depth 8 \
#     gray:${trajdir}r.char gray:${trajdir}g.char gray:${trajdir}b.char \
#     -channel RBG -combine -gamma 1.2 -rotate 90 ${trajdir}${imgname}rgb.png

echo --------- Opening image --------
sxiv ${trajdir}${imgname}hints.png ${trajdir}${imgname}gray.png
