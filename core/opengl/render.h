#ifndef RENDER_H_
#define RENDER_H_

#include "../budack_core.h"
#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>

#define MAX_UINT 4294967295
#define MAX_RENDER_SIZE 1000

typedef struct {
  GLFWwindow *window;
  Pts_msg *recbuff;
  double dx;
  uint32_t *R, *G, *B;
  unsigned int width, height;
  unsigned int shader_program, compute_program;
  uint32_t *maxv;
  unsigned int maxv_ssbo, max_loc;
  unsigned int R_image_ID, Runit, G_image_ID, Gunit, B_image_ID, Bunit;
  unsigned int recbuff_ID, recbuff_unit, recbuff_length, recbuff_ssbo;
  unsigned int VAO;
  unsigned int VBO;

} Render_object;

void render_init(Render_object *rdr_obj);

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(Render_object *rdr_obj,
                                            void *fargs),
                void *fargs);

int render_finalize(Render_object *rdr_obj);

void set_image2D(unsigned int unit, unsigned int *imageID, unsigned int width,
                 unsigned int height, uint32_t *img_data);

void set_ssbo(void *data, size_t size, unsigned int unit, unsigned int *ssbo);

#endif // RENDER_H_
