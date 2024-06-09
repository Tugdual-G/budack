#include "tiff_images.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <tiffio.h>

int write_tiff_16bitsRGB(const char *fname, uint16_t *R, uint16_t *G,
                         uint16_t *B, unsigned width, unsigned height) {

  size_t size = width * height;

  uint16_t *RGB = (uint16_t *)malloc(3 * size * sizeof(uint16_t));
  for (size_t k = 0; k < size; ++k) {
    RGB[3 * k] = R[k];
    RGB[3 * k + 1] = G[k];
    RGB[3 * k + 2] = B[k];
  }

  // Open the TIFF file for writing
  TIFF *tif = TIFFOpen(fname, "w");

  if (!tif) {
    fprintf(stderr, "Failed to open %s for writing\n", fname);
    return 1;
  }

  // Set the TIFF fields
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32_t)width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32_t)height);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3); // 3 channels (RGB)
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 16);  // 16 bits per channel
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
  TIFFSetField(tif, TIFFTAG_SOFTWARE, "Budack");
  /* TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, ""); */
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

  // Write the image data to the TIFF file
  for (unsigned row = 0; row < height; row++) {
    if (TIFFWriteScanline(tif, &RGB[row * width * 3], (uint32_t)row, 0) < 0) {
      fprintf(stderr, "Failed to write scanline %u\n", row);
      TIFFClose(tif);
      free(RGB);
      return 1;
    }
  }

  // Close the TIFF file
  TIFFClose(tif);
  free(RGB);

  printf("16 bits per channel RGB TIFF file created successfully, %s\n", fname);

  return 0;
}

void normalize_uint_to_16bits(unsigned int *in, uint16_t *out, size_t n) {
  double max = 0;
  for (size_t k = 0; k < n; k++) {
    if (*(in + k) > max) {
      max = *(in + k);
    }
  }
  for (size_t k = 0; k < n; k++) {
    *(out + k) = 65535.0 * (double)*(in + k) / max;
  }
}
