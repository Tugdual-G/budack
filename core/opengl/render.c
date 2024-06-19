#include "render.h"
/* #include "../master.h" */
#include "compileShader.h"
#include "display_window.h"
#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_MOVE_DIST 100

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void render_init(Render_object *rdr_obj) {

  // Init Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  rdr_obj->window =
      glfwCreateWindow(rdr_obj->width, rdr_obj->height, "Budack", NULL, NULL);
  if (rdr_obj->window == NULL) {
    printf("Failed to create GLFW rdr_obj->window\n");
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(rdr_obj->window);
  glfwSetWindowUserPointer(rdr_obj->window, rdr_obj);
  glfwSetKeyCallback(rdr_obj->window, keyboard_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    exit(1);
  }

  // Compile shaders
  unsigned int vertexShader = compileVertexShader("shaders/vertexShader.glsl");
  unsigned int fragmentShader =
      compileFragmentShader("shaders/rgbassemble.glsl");
  rdr_obj->shader_program = linkShaders(vertexShader, fragmentShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(rdr_obj->shader_program);
  rdr_obj->maxval_loc = glGetUniformLocation(rdr_obj->shader_program, "maxval");

  glfwSetFramebufferSizeCallback(rdr_obj->window, framebuffer_size_callback);
  keep_aspect_ratio(rdr_obj->window, rdr_obj->width, rdr_obj->height);

  // Define a list of points
  float vertices[] = {
      // points coord     // texture coords
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Point 1
      1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // Point 2
      1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // Point 3
      -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // Point 4
  };

  unsigned int square_idx[] = {
      0, 1, 2, 2, 3, 0,
  };
  unsigned int EBO;

  glGenVertexArrays(1, &rdr_obj->VAO);
  glGenBuffers(1, &rdr_obj->VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(rdr_obj->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, rdr_obj->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Square EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_idx), square_idx,
               GL_STATIC_DRAW);

  // Position attrib
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Texture attrib
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Loading texture
  set_image2D(rdr_obj->Runit, &rdr_obj->R_image_ID, rdr_obj->width,
              rdr_obj->height, rdr_obj->R);
  set_image2D(rdr_obj->Gunit, &rdr_obj->G_image_ID, rdr_obj->width,
              rdr_obj->height, rdr_obj->G);
  set_image2D(rdr_obj->Bunit, &rdr_obj->B_image_ID, rdr_obj->width,
              rdr_obj->height, rdr_obj->B);
  glCheckError();
}

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(uint16_t *R, uint16_t *G,
                                            uint16_t *B, void *fargs),
                void *fargs) {

  int flag = 1;
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glUseProgram(rdr_obj->shader_program);

  while (!glfwWindowShouldClose(rdr_obj->window) && flag) {
    keep_aspect_ratio(rdr_obj->window, rdr_obj->width, rdr_obj->height);
    processInput(rdr_obj->window);
    glClear(GL_COLOR_BUFFER_BIT);

    flag = data_update_function(rdr_obj->R, rdr_obj->G, rdr_obj->B, fargs);
    glUniform3ui(rdr_obj->maxval_loc, rdr_obj->Rmax, rdr_obj->Gmax,
                 rdr_obj->Bmax);

    glTextureSubImage2D(rdr_obj->R_image_ID, 0, 0, 0, rdr_obj->width,
                        rdr_obj->height, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                        rdr_obj->R);
    glTextureSubImage2D(rdr_obj->G_image_ID, 0, 0, 0, rdr_obj->width,
                        rdr_obj->height, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                        rdr_obj->G);
    glTextureSubImage2D(rdr_obj->B_image_ID, 0, 0, 0, rdr_obj->width,
                        rdr_obj->height, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
                        rdr_obj->B);

    // render container
    glBindVertexArray(rdr_obj->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(rdr_obj->window);
    glfwPollEvents();
  }

  glCheckError();
  return 0;
}

int render_finalize(Render_object *rdr_obj) {
  // Cleanup
  glDeleteVertexArrays(1, &(*rdr_obj).VAO);
  glDeleteBuffers(1, &(*rdr_obj).VBO);
  glfwTerminate();
  return 0;
}

void set_image2D(unsigned int unit, unsigned int *imageID, unsigned int width,
                 unsigned int height, uint16_t *img_data) {

  glGenTextures(1, imageID);
  glBindTexture(GL_TEXTURE_2D, *imageID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, width, height, 0, GL_RED_INTEGER,
               GL_UNSIGNED_SHORT, img_data);
  glBindImageTexture(unit, *imageID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
  glCheckError();
}

void keyboard_callback(GLFWwindow *window, int key,
                       __attribute__((unused)) int scancode, int action,
                       __attribute__((unused)) int mods) {

  Render_object *rdr = glfwGetWindowUserPointer(window);
  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_RIGHT:
      if (*rdr->j_ll + KEY_MOVE_DIST + rdr->width < rdr->max_width) {
        *rdr->j_ll += KEY_MOVE_DIST;
      } else {
        *rdr->j_ll = rdr->max_width - rdr->width;
      }
      break;

    case GLFW_KEY_LEFT:
      if (*rdr->j_ll - KEY_MOVE_DIST > 0) {
        *rdr->j_ll -= KEY_MOVE_DIST;
      } else {
        *rdr->j_ll = 0;
      }
      break;

    case GLFW_KEY_DOWN:
      if (*rdr->i_ll - KEY_MOVE_DIST > 0) {
        *rdr->i_ll -= KEY_MOVE_DIST;
      } else {
        *rdr->i_ll = 0;
      }
      break;

    case GLFW_KEY_UP:
      if (*rdr->i_ll + KEY_MOVE_DIST + rdr->height < rdr->max_height) {
        *rdr->i_ll += KEY_MOVE_DIST;
      } else {
        *rdr->j_ll = rdr->max_height - rdr->height;
      }
      break;
    }
  }
}

GLenum glCheckError_(const char *file, int line) {
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    switch (errorCode) {
    case GL_INVALID_ENUM: {
      printf(" ERROR : INVALID_ENUM , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_VALUE: {
      printf(" ERROR : INVALID_VALUE , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_OPERATION: {
      printf(" ERROR : INVALID_OPERATION , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_STACK_OVERFLOW: {
      printf(" ERROR : STACK_OVERFLOW , file: %s  ,line: %i \n", file, line);
      break;
    }
    case GL_STACK_UNDERFLOW: {
      printf(" ERROR : STACK_UNDERFLOW , file: %s ,line: %i \n", file, line);
      break;
    }
    case GL_OUT_OF_MEMORY: {
      printf(" ERROR : OUT_OF_MEMORY , file: %s ,line: %i \n", file, line);
      break;
    }
    case GL_INVALID_FRAMEBUFFER_OPERATION: {
      printf(" ERROR : INVALID_FRAMEBUFFER_OPERATION , file: %s ,line: %i \n",
             file, line);
      break;
    }
    }
  }
  return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)
