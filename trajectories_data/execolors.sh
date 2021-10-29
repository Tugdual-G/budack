#!/usr/bin/env bash
#
dir=/home/tugdual/Documents/Programmation/C/budac/
budackc=${dir}budack.c
budack=${dir}budack
trajdir=${dir}trajectories_data/
traj=${trajdir}traj.char
nx=1000
ny=832
startingpts=1000
density=400000

# Image sizes (nx x ny):
#
# 1000x832
# 2000x1666
# 8000x6666

echo ------------- 1 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 2000 1000 $startingpts $density
mv $traj ${trajdir}r.char

echo ------------- 2 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 1000 500 $startingpts $density
mv $traj ${trajdir}g.char

echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 500 100 $startingpts $density
mv $traj ${trajdir}b.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
    R:${trajdir}r.char G:${trajdir}g.char B:${trajdir}b.char \
     -combine -gamma 2 -rotate 90 ${trajdir}rgbuhd2000_100.png

echo ------------- 1 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 2000 500 $startingpts $density
mv $traj ${trajdir}r2.char

echo ------------- 2 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 500 100 $startingpts $density
mv $traj ${trajdir}g2.char

echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 500 100 $startingpts $density
mv $traj ${trajdir}b2.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
    R:${trajdir}r2.char G:${trajdir}g2.char B:${trajdir}b2.char \
     -combine -gamma 2 -rotate 90 ${trajdir}rgbuhd2000_fa100.png




echo ------------- 1 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 1000 800 $startingpts $density
mv $traj ${trajdir}r3.char

echo ------------- 2 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 900 700 $startingpts $density
mv $traj ${trajdir}g3.char

echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 500 100 $startingpts $density
mv $traj ${trajdir}b3.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
    R:${trajdir}r3.char G:${trajdir}g3.char B:${trajdir}b3.char \
     -combine -gamma 2 -rotate 90 ${trajdir}rgbuhd2000_fa100f.png




echo ------------- 1 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 500 200 $startingpts $density
mv $traj ${trajdir}r4.char

echo ------------- 2 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 300 100 $startingpts $density
mv $traj ${trajdir}g4.char

echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 90 0 $startingpts $density
mv $traj ${trajdir}b4.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
    R:${trajdir}r4.char G:${trajdir}g4.char B:${trajdir}b4.char \
     -combine -gamma 2 -rotate 90 ${trajdir}rgbuhd2000_fasaf100f.png



echo ------------- 2 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 3000 2000 $startingpts $density
mv $traj ${trajdir}g5.char

echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 90 0 $startingpts $density
mv $traj ${trajdir}b5.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
    R:${trajdir}g5.char B:${trajdir}b5.char \
     -combine -gamma 2 -rotate 90 ${trajdir}rgbusfhd2000_fasaf100f.png



echo ------------- 3 -------------
mpiexec --mca opal_warn_on_missing_libcuda 0 $budack $nx 9000 4000 $startingpts $density
mv $traj ${trajdir}b6.char

echo --------- Creating image --------
magick convert -size ${nx}x${ny} -depth 8 \
     GRAY:${trajdir}b6.char \
      -gamma 1.5 -rotate 90 ${trajdir}rgbusfhd2dg000_fasaf100f.png

echo --------- Opening image --------
sxiv ${trajdir}rgbuhd.png
