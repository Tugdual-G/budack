#include "compileShader.h"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

unsigned int compileVertexShader(const char *sourcefname) {

  FILE *fp = fopen(sourcefname, "rb+");
  if (!fp) {
    printf("error: cannot opern vertex shader source file \n");
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t lengthOfFile = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *source = (char *)malloc(lengthOfFile + 1);
  if (!source) {
    printf("error source \n");
    exit(1);
  }
  if (fread(source, 1, lengthOfFile, fp) < lengthOfFile) {
    printf("error loading vertex shader source \n");
    exit(1);
  }
  source[lengthOfFile] = '\0';
  const char *s = source;

  int success;
  char infoLog[512] = {'\0'};
  unsigned int v_shader;
  // Compile shaders
  v_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v_shader, 1, &s, NULL);
  glCompileShader(v_shader);
  glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(v_shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  return v_shader;
}

unsigned int compileFragmentShader(const char *sourcefname) {
  FILE *fp = fopen(sourcefname, "rb+");
  if (!fp) {
    printf("error: cannot open fragment shader source file \n");
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  size_t lengthOfFile = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *source = (char *)malloc(lengthOfFile + 1);
  if (!source) {
    printf("error source \n");
    exit(1);
  }
  if (fread(source, 1, lengthOfFile, fp) < lengthOfFile) {
    printf("error loading fragment shader source \n");
    exit(1);
  }
  source[lengthOfFile] = '\0';
  const char *s = source;

  int success;
  char infoLog[512] = {'\0'};
  unsigned int f_shader;

  f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f_shader, 1, &s, NULL);
  glCompileShader(f_shader);
  glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(f_shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  fclose(fp);
  return f_shader;
}

unsigned int linkShaders(unsigned int v_shader, unsigned int f_shader) {
  unsigned int shaderProgram;
  int success;
  char infoLog[512] = {'\0'};
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, v_shader);
  glAttachShader(shaderProgram, f_shader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n");
    printf("%s\n", infoLog);
  }
  return shaderProgram;
}
