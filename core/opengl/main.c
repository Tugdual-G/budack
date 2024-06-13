#include "render.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int readfile_uint8(char *filename, size_t size, uint8_t *buffer) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    printf("Error opening file \n");
    return 0;
  }
  if (fread(buffer, sizeof(uint8_t), size, fp) < size) {
    printf("Error reading file \n");
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 1;
}

typedef struct {
  unsigned int heigth;
  unsigned int width;
  unsigned int i, j;
} Fargs;

int callback(uint8_t *data, void *fargs) {
  Fargs *args = (Fargs *)fargs;
  args->j %= args->width - 5;
  args->i %= args->heigth;
  for (unsigned char k = 0; k < 5; ++k) {
    data[3 * (args->i * args->width + args->j + k)] = 255;
  }
  args->i += 2;
  args->j += 3;
  sleep(1);

  return 1;
}

int main() {
  // load and generate the texture
  int width = 1000, heigth = 832;
  int nchannels = 3;
  size_t size = width * heigth;
  uint8_t *data = (uint8_t *)malloc(size * nchannels * sizeof(uint8_t));

  char fname[] = "rgb.uint8";
  if (!readfile_uint8(fname, size * 3, data)) {
    printf("Error loading file %s", fname);
    return 1;
  }
  Render_object rdr_obj = render_init(data, width, heigth, nchannels);
  Fargs fargs = {.heigth = heigth, .width = width, .i = 0, .j = 0};
  render_loop(rdr_obj, data, width, heigth, callback, &fargs);
  render_finalize(&rdr_obj);
  free(data);
  return 0;
}
