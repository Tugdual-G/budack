/*
** Image output functions.
*/
#ifndef TIFF_IMAGES_H_
#define TIFF_IMAGES_H_

#include <stdint.h>
#include <stdlib.h>

int write_tiff_16bitsRGB(const char *fname, uint16_t *R, uint16_t *G,
                         uint16_t *B, unsigned width, unsigned height);
/*
** Writes a 16 bits per channels tiff image to disk.
*/

int write_tiff_8bits_grayscale(const char *fname, uint8_t *gray_scale,
                               unsigned width, unsigned height);
/*
** Writes a 8 bits grayscale image to disk.
*/

void normalize_16bits(uint16_t *inout, size_t n);

void normalize_32_to_16bits(unsigned int *in, uint16_t *out, size_t n);
#endif // TIFF_IMAGES_H_
