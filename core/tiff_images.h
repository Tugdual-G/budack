#ifndef TIFF_IMAGES_H_
#define TIFF_IMAGES_H_

#include <stdint.h>
#include <stdlib.h>

int write_tiff_16bitsRGB(const char *fname, uint16_t *R, uint16_t *G,
                         uint16_t *B, unsigned width, unsigned height);

int write_tiff_16bits_grayscale(const char *fname, uint8_t *gray_scale,
                                unsigned width, unsigned height);

void normalize_uint_to_16bits(unsigned int *in, uint16_t *out, size_t n);
#endif // TIFF_IMAGES_H_
