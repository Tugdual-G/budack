/*
** The slaves processes finds starting points with good trajectory
** property (long, divergent ...).
** Then the points are sent to the master process.
*/
#ifndef SLAVE_H_
#define SLAVE_H_
#include "budack_core.h"

// All of the slaves process actions will be called from this function
int slave(int world_size, int rank, Param param, const double x_b[2]);
#endif // SLAVE_H_
