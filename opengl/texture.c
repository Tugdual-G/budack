#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

const char *vertexShaderSource =
    "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec4 vertexColor;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "   vertexColor = vec4(1, (1+aPos.x)/2.0, (1+aPos.y)/2.0, 1.0);"
    "   TexCoord = aTexCoord;\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 460 core\n"
    "out vec4 FragColor;\n"
    "in vec4 vertexColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D ourTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(ourTexture, TexCoord);\n"
    "}\n";

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

int readfile_uint8(char *filename, size_t size, uint8_t *buffer) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    printf("Error opening file \n");
    return 1;
  }
  if (fread(buffer, sizeof(uint8_t), size, fp) < size) {
    printf("Error reading file \n");
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
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

int main() {

  // Init Window
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return 1;
  }

  int width = 1000, heigth = 832;
  keep_aspect_ratio(window, width, heigth);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  printf("windows init \n");

  // Compile shaders
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  int success;
  char infoLog[512] = {'\0'};
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }

  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  printf("Shaders compiled \n");

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
  unsigned int VAO, VBO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set the texture wrapping/filtering options (on the currently bound
  // texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // load and generate the texture
  int nrChannels = 3;
  size_t size = width * heigth;
  uint8_t *data = (uint8_t *)malloc(size * nrChannels * sizeof(uint8_t));
  if (!readfile_uint8("rgb.uint8", 3 * size, data)) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("failed to load texture \n");
  }
  int i = 0, j = 0;

  while (!glfwWindowShouldClose(window)) {
    j %= width;
    i %= heigth;
    data[3 * (i * width + j)] = 255;
    i += 2;
    j += 3;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);

    processInput(window);
    keep_aspect_ratio(window, width, heigth);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);

    // render container
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* /\* // Draw the points *\/ */
    /* glPointSize(20.0f); // Set point size */
    /* glDrawArrays(GL_POINTS, 0, 5); */

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();

  free(data);
  return 0;
}
