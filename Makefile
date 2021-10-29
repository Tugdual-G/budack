##
# Budack
#
# @file
# @version 0.1
build:
	mpicc budack.c -o budack -lm -Wall
run:
	mpiexec --mca opal_warn_on_missing_libcuda 0 ./budack
# end
