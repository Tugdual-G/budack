#ifndef RENDER_H_
#define RENDER_H_

#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>

#define MAX_UINT 4294967295
#define MAX_RENDER_SIZE 1000

typedef struct {
  GLFWwindow *window;
  unsigned int shader_program, compute_program;
  unsigned int R_image_ID, Runit, G_image_ID, Gunit, B_image_ID, Bunit;
  unsigned int width, height;
  unsigned int VAO, VBO;
  uint32_t *R, *G, *B;
  uint32_t Rmax, Gmax, Bmax;
  unsigned int maxv_loc;

} Render_object;

void render_init(Render_object *rdr_obj);

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(uint32_t *R, uint32_t *G,
                                            uint32_t *B, void *fargs),
                void *fargs);

int render_finalize(Render_object *rdr_obj);

void set_image2D(unsigned int unit, unsigned int *imageID, unsigned int width,
                 unsigned int height, uint32_t *img_data);
#endif // RENDER_H_
