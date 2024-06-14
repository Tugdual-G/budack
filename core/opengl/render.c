#include "render.h"
#include "compileShader.h"
#include "display_window.h"
#include "include/glad/glad.h" // glad should be included before glfw3
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

Render_object render_init(uint8_t *data, unsigned int width,
                          unsigned int height, char nchannels) {

  Render_object rdr_obj;
  // Init Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  rdr_obj.window = glfwCreateWindow(width, height, "Budack", NULL, NULL);
  if (rdr_obj.window == NULL) {
    printf("Failed to create GLFW rdr_obj.window\n");
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(rdr_obj.window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    exit(1);
  }

  keep_aspect_ratio(rdr_obj.window, width, height);
  glfwSetFramebufferSizeCallback(rdr_obj.window, framebuffer_size_callback);

  // Compile shaders
  unsigned int vertexShader = compileVertexShader("vertexShader.glsl");

  unsigned int fragmentShader = compileFragmentShader("fragmenShader.glsl");

  rdr_obj.shader_program = linkShaders(vertexShader, fragmentShader);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

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

  glGenVertexArrays(1, &rdr_obj.VAO);
  glGenBuffers(1, &rdr_obj.VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(rdr_obj.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, rdr_obj.VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Square EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_idx), square_idx,
               GL_STATIC_DRAW);

  // Position attrib
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Vertex attrib
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Loading texture
  glGenTextures(1, &rdr_obj.texture);
  glBindTexture(GL_TEXTURE_2D, rdr_obj.texture);
  // set the texture wrapping/filtering options (on the currently bound
  // texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB, width, height);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  return rdr_obj;
}

int render_loop(Render_object rdr_obj, uint8_t *data, unsigned int width,
                unsigned int height,
                int (*data_update_function)(uint8_t *data, void *fargs),
                void *fargs) {

  int flag;
  while (!glfwWindowShouldClose(rdr_obj.window) && flag) {
    keep_aspect_ratio(rdr_obj.window, width, height);
    processInput(rdr_obj.window);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    flag = data_update_function(data, fargs);

    glBindTexture(GL_TEXTURE_2D, rdr_obj.texture);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB,
                    GL_UNSIGNED_BYTE, data);

    // render container
    glUseProgram(rdr_obj.shader_program);
    glBindVertexArray(rdr_obj.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(rdr_obj.window);
    glfwPollEvents();
  }

  return 0;
}

int render_finalize(Render_object *rdr_obj) {
  // Cleanup
  glDeleteVertexArrays(1, &(*rdr_obj).VAO);
  glDeleteBuffers(1, &(*rdr_obj).VBO);
  glfwTerminate();
  return 0;
}
