##
# Budack
#
# @file
# @version 0.1
build:
	mpicc budack_core.c main.c tiff_images.c -o budack -lm -ltiff -Wall -O2
run:
	mpiexec --mca opal_warn_on_missing_libcuda 0 ./budack
# end
