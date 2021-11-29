##
# Budack
#
# @file
# @version 0.1
build:
	# mpicc -g -O3 -c budack_core.c
	# mpicc budack_core.o budack.c -o budack -lm -Wall -O2
	mpicc budack_core.c budack.c -o budack -lm -Wall -O2
	# mpicc gamma.c -o gamma -lm -Wall
run:
	mpiexec --mca opal_warn_on_missing_libcuda 0 ./budack
# end
