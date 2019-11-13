#pragma once

#include "Window.h"
#include "Camera.h"
#include "mesh.h"

int N = 1000;
int P = 20;
int seed = 10;

#include "PlanetGen.hpp"

void TransformToMesh(GLfloat * &vertices, GLuint * &indices, const Isocahedron * planetMesh);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);