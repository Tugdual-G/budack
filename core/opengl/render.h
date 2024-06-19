#ifndef RENDER_H_
#define RENDER_H_

#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>

#define MAX_UINT 4294967295
#define MAX_UINT16 65535
#define MAX_RENDER_SIZE 1000

typedef struct {
  GLFWwindow *window;
  unsigned int shader_program, compute_program;
  unsigned int R_image_ID, Runit, G_image_ID, Gunit, B_image_ID, Bunit;
  unsigned int width, height, *i_ll, *j_ll, max_width, max_height;
  unsigned int VAO, VBO, maxval_loc;
  uint16_t *R, *G, *B, Rmax, Gmax, Bmax;

} Render_object;

void render_init(Render_object *rdr_obj);

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(uint16_t *R, uint16_t *G,
                                            uint16_t *B, void *fargs),
                void *fargs);

int render_finalize(Render_object *rdr_obj);

void set_image2D(unsigned int unit, unsigned int *imageID, unsigned int width,
                 unsigned int height, uint16_t *img_data);

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action,
                       int mods);
#endif // RENDER_H_
