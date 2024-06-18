#include "display_window.h"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

void framebuffer_size_callback(__attribute__((unused)) GLFWwindow *window,
                               int width, int height) {
  glViewport(0, 0, width, height);
}

void keep_aspect_ratio(GLFWwindow *window, int width, int heigth) {
  int w_width, w_heigth, gl_width, gl_heigth;
  double aspect_w, aspect;
  glfwGetWindowSize(window, &w_width, &w_heigth);
  aspect = heigth / (double)width;
  aspect_w = w_heigth / (double)w_width;
  int x0 = 0, y0 = 0;
  if (aspect < aspect_w) {
    gl_width = w_width;
    gl_heigth = w_width * aspect;
    y0 = (w_heigth - gl_heigth) / 2;
  } else {
    gl_width = w_heigth / aspect;
    gl_heigth = w_heigth;
    x0 = (w_width - gl_width) / 2;
  }

  glViewport(x0, y0, gl_width, gl_heigth);
}
