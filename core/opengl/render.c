#include "render.h"
/* #include "../master.h" */
#include "compileShader.h"
#include "display_window.h"
#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

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
  unsigned int vertexShader = compileVertexShader("shaders/vertexShader.glsl");
  unsigned int fragmentShader =
      compileFragmentShader("shaders/rgbassemble.glsl");
  rdr_obj->shader_program = linkShaders(vertexShader, fragmentShader);
  rdr_obj->compute_program = computeShaderProgram("shaders/computeshader.glsl");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glUseProgram(rdr_obj->shader_program);
  unsigned int Rmaxid = glGetUniformLocation(rdr_obj->shader_program, "Rmax");
  unsigned int Gmaxid = glGetUniformLocation(rdr_obj->shader_program, "Gmax");
  unsigned int Bmaxid = glGetUniformLocation(rdr_obj->shader_program, "Bmax");
  glUniform1ui(Rmaxid, rdr_obj->Rmax);
  glUniform1ui(Gmaxid, rdr_obj->Gmax);
  glUniform1ui(Bmaxid, rdr_obj->Bmax);

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

  /* glUseProgram(rdr_obj->compute_program); */
  /* glDispatchCompute((unsigned int)rdr_obj->width, (unsigned
   * int)rdr_obj->height, 1); */
  /* // make sure writing to image has finished before read */
  /* glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); */
  glCheckError();
}

int render_loop(Render_object *rdr_obj,
                int (*data_update_function)(uint32_t *R, uint32_t *G,
                                            uint32_t *B, void *fargs),
                void *fargs) {

  int flag = 1;
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glUseProgram(rdr_obj->shader_program);
  unsigned int Rmaxid = glGetUniformLocation(rdr_obj->shader_program, "Rmax");
  unsigned int Gmaxid = glGetUniformLocation(rdr_obj->shader_program, "Gmax");
  unsigned int Bmaxid = glGetUniformLocation(rdr_obj->shader_program, "Bmax");
  glUniform1ui(Rmaxid, rdr_obj->Rmax);
  glUniform1ui(Gmaxid, rdr_obj->Gmax);
  glUniform1ui(Bmaxid, rdr_obj->Bmax);
  /* printf("Rmax = %u\n", rdr_obj->Rmax); */
  /* printf("Gmax = %u\n", rdr_obj->Gmax); */
  /* printf("Bmax = %u\n", rdr_obj->Bmax); */

  while (!glfwWindowShouldClose(rdr_obj->window) && flag) {
    keep_aspect_ratio(rdr_obj->window, rdr_obj->width, rdr_obj->height);
    /* processInput(rdr_obj->window); */
    glClear(GL_COLOR_BUFFER_BIT);

    flag = data_update_function(rdr_obj->R, rdr_obj->G, rdr_obj->B, fargs);
    glUniform1ui(Rmaxid, rdr_obj->Rmax);
    glUniform1ui(Gmaxid, rdr_obj->Gmax);
    glUniform1ui(Bmaxid, rdr_obj->Bmax);

    glBindTexture(GL_TEXTURE_2D, rdr_obj->R_image_ID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rdr_obj->width, rdr_obj->height,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, rdr_obj->R);
    glBindTexture(GL_TEXTURE_2D, rdr_obj->G_image_ID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rdr_obj->width, rdr_obj->height,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, rdr_obj->G);
    glBindTexture(GL_TEXTURE_2D, rdr_obj->B_image_ID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rdr_obj->width, rdr_obj->height,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, rdr_obj->B);

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
                 unsigned int height, uint32_t *img_data) {

  glGenTextures(1, imageID);
  glBindTexture(GL_TEXTURE_2D, *imageID);

  /* glCheckError(); */
  /* glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height); */
  /* glCheckError(); */

  /* glBindTexture(GL_TEXTURE_2D, 0); */
  /* printf("width %u height %u , GL_MAX_TEXTURE_SIZE %u \n", width, height, */
  /*        GL_MAX_TEXTURE_SIZE); */

  /* glBindTexture(GL_TEXTURE_2D, *imageID); */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER,
               GL_UNSIGNED_INT, img_data);

  /* glCheckError(); */
  /* glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, */
  /*                 GL_UNSIGNED_INT, img_data); */

  glBindImageTexture(unit, *imageID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
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
