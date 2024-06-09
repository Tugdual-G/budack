##
# Budack
#
# @file
# @version 0.1
build:
	mpicc core/budack_core.c core/budack.c core/tiff_images.c -o core/budack -lm -ltiff -Wall -O2
	mpicc core/gamma.c -o core/gamma -lm -Wall
run:
	mpiexec --mca opal_warn_on_missing_libcuda 0 ./budack
# end
