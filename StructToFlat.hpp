#pragma once

#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <gl/glew.h>
#include <gl/glfw3.h>
#endif

#include <vector>
#include "Planet.hpp"

std::vector<float> vec3ToFlat(std::vector<glm::vec3> orig);
std::vector<double> vec2ToDouble(std::vector<glm::vec2> * points);

void meshToFlat(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const TriangleMesh * planetMesh);
void quadToFlat(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const QuadGeometry * planetMesh);
void voronoiToFlat(std::vector<GLfloat> &vertices, const Voronoi * planetMesh);