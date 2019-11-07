#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>
#include  <glm/glm.hpp>

GLFWwindow *  InitWindow(int * settings);
void Update(GLFWwindow * window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//Загрузка шейдеров
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
std::string ReadShaderCode(const char * path);
int CompileShader(std::string shaderCode, const char * path, GLuint ID);