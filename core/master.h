#ifndef MASTER_H_
#define MASTER_H_
#include "budack_core.h"

int master(int world_size, Param param, double a[2], double b[2]);

void recieve_and_draw(uint32_t *R, uint32_t *G, uint32_t *B, double a[2],
                      double b[2], unsigned int nx, unsigned int ny,
                      int world_size);
#endif // MASTER_H_
