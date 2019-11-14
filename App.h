#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>
#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include "Camera.h"
#include "mesh.h"
#include "ColorMap.h"
#include "Shader.hpp"

int N = 1000;
int P = 20;
int seed = 10;

#include "PlanetGen.hpp"

GLFWwindow *  InitWindow(int * settings);
void TransformToMesh(GLfloat * &vertices, GLuint * &indices, const QuadGeometryV * planetMesh);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);