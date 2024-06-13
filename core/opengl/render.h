#ifndef RENDER_H_
#define RENDER_H_

#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow *window;
  unsigned int shader_program;
  unsigned int texture;
  unsigned int VAO;
  unsigned int VBO;

} Render_object;

Render_object render_init(uint8_t *data, unsigned int width,
                          unsigned int heigth, char nchannels);

int render_loop(Render_object rdr_obj, uint8_t *data, unsigned int width,
                unsigned int heigth,
                int (*data_update_function)(uint8_t *data, void *fargs),
                void *fargs);

int render_finalize(Render_object *rdr_obj);
#endif // RENDER_H_
