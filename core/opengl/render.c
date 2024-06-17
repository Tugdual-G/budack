#include "render.h"
#include "compileShader.h"
#include "display_window.h"
#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    exit(1);
  }

  // Compile shaders
  unsigned int vertexShader = compileVertexShader("shaders/vertex_shader.glsl");
  unsigned int fragmentShader =
      compileFragmentShader("shaders/gather_rgb_fragment.glsl");
  rdr_obj->shader_program = linkShaders(vertexShader, fragmentShader);
  rdr_obj->compute_program =
      computeShaderProgram("shaders/iterate_compshader.glsl");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(rdr_obj->shader_program);

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

  // Loading starting points
  set_ssbo(rdr_obj->recbuff, rdr_obj->recbuff_length * sizeof(Pts_msg),
           rdr_obj->recbuff_unit, &rdr_obj->recbuff_ssbo);

  glUseProgram(rdr_obj->compute_program);
  unsigned int dx_loc = 999999, max_loc;
  dx_loc = glGetUniformLocation(rdr_obj->compute_program, "dx");
  glUniform1f(dx_loc, (float)(rdr_obj->dx));

  glUseProgram(rdr_obj->shader_program);
  max_loc = glGetUniformLocation(rdr_obj->shader_program, "maxval");
  glUniform3ui(max_loc, (unsigned)100, (unsigned)70, (unsigned)40);
  glCheckError();
}

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(Render_object *rdr_obj,
                                            void *fargs),
                void *fargs) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glUseProgram(rdr_obj->shader_program);
  glCheckError();
  int flag = 1;
  int it = 0;
  while (!glfwWindowShouldClose(rdr_obj->window) && flag && it < 500) {
    keep_aspect_ratio(rdr_obj->window, rdr_obj->width, rdr_obj->height);
    processInput(rdr_obj->window);
    glClear(GL_COLOR_BUFFER_BIT);

    glCheckError();

    flag = data_update_function(rdr_obj, fargs);
    glCheckError();

    glUseProgram(rdr_obj->shader_program);
    // render container
    glBindVertexArray(rdr_obj->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glfwSwapBuffers(rdr_obj->window);
    glfwPollEvents();
    // usleep(500000);
    ++it;
  }

  glCheckError();
  return 0;
}

int render_finalize(Render_object *rdr_obj) {
  // Cleanup
  glDeleteVertexArrays(1, &(*rdr_obj).VAO);
  glDeleteBuffers(1, &(*rdr_obj).VBO);
  glDeleteBuffers(1, &(*rdr_obj).VBO);
  glfwTerminate();
  return 0;
}

void set_image2D(unsigned int unit, unsigned int *imageID, unsigned int width,
                 unsigned int height, uint32_t *img_data) {

  glGenTextures(1, imageID);
  glBindTexture(GL_TEXTURE_2D, *imageID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER,
               GL_UNSIGNED_INT, img_data);

  glBindImageTexture(unit, *imageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
  glBindTexture(GL_TEXTURE_2D, 0);
  glCheckError();
}

void set_ssbo(Pts_msg *data, size_t size, unsigned int unit,
              unsigned int *ssbo) {

  glGenBuffers(1, ssbo);
  printf("ssbo = %u \n ", *ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, *ssbo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data,
               GL_DYNAMIC_DRAW); // sizeof(data) only works for statically sized
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, *ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
  glCheckError();
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
