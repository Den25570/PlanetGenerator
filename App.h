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
#include "VAO.hpp"

#include "Planet.hpp"

GLFWwindow *  InitWindow(int * settings);
void drawTriangleMesh(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const TriangleMesh * planetMesh);
void drawIndexedTriangles(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const QuadGeometryV * planetMesh);
void drawTriangles(std::vector<GLfloat> &vertices, const Voronoi * planetMesh);
std::vector<vec3> drawPlateBoundaries(std::vector<GLfloat> &vertices, TriangleMesh * mesh, Map * map);
std::vector<vec3> drawPlateVectors(std::vector<GLfloat> &vertices, TriangleMesh * mesh, Map * map);
std::vector<vec3> drawRivers(std::vector<GLfloat> &vertices, TriangleMesh * mesh, Map * map);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);