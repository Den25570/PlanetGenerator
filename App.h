#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>


#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>
#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include <ctime>

#include "Camera.h"
#include "ColorMap.h"
#include "Shader.hpp"

#include "Planet.hpp"

int N = 5000;
int P = 20;
int seed = 12300;

GLFWwindow *  InitWindow(int * settings);
void drawIndexedTriangles(GLfloat * &vertices, GLuint * &indices, const QuadGeometryV * planetMesh);
void drawTriangles(GLfloat * &vertices, const Voronoi * planetMesh);
std::vector<vec3> drawPlateBoundaries(GLfloat * &vertices, TriangleMesh * mesh, Map * map);
std::vector<vec3> drawPlateVectors(GLfloat * &vertices, TriangleMesh * mesh, Map * map);
std::vector<vec3> drawRivers(GLfloat * &vertices, TriangleMesh * mesh, Map * map);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);