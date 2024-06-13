#ifndef DISPLAY_H_
#define DISPLAY_H_
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>

void processInput(GLFWwindow *window);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

int readfile_uint8(char *filename, size_t size, uint8_t *buffer);
void keep_aspect_ratio(GLFWwindow *window, int width, int heigth);

#endif // DISPLAY_H_
