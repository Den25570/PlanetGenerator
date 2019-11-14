#pragma once

#include "mesh.h"
#include "ExtendedMath.hpp"
#include "delaunator.hpp"
#include "SimplexNoise.h"
#include "PlanetData.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <cstdlib>

#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>
#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

using namespace delaunator;

void pushCentroidOfTriangle(std::vector<size_t> * out, size_t ax, size_t ay, size_t az, size_t bx, size_t by, size_t bz, size_t cx, size_t cy, size_t cz);


float fbm_noise(float nx, float ny, float nz);

std::vector<size_t> assignDistanceField(TriangleMesh * mesh, std::vector<size_t> seeds_r, std::vector<size_t> stop_r);
void assignRegionElevation(TriangleMesh * mesh, Map * map);
void assignTriangleValues(TriangleMesh* mesh, Map* map);
void assignDownflow(TriangleMesh* mesh, Map* map, std::queue<int>* _queue);
void assignFlow(TriangleMesh* mesh, Map* map);

Map generateMesh(TriangleMesh orig_mesh, int N, int P, int seed, QuadGeometry * quadg);
void generateMap(TriangleMesh * mesh, Map * map, QuadGeometry * quadGeometry, int N, int P, int seed);
TriangleMesh generateDelanuaySphere(std::vector<vec3>* verticles);
void addSouthPoleToMesh(uint southPoleId, Delaunator * delanuay, std::vector<vec3> * p);

std::vector<size_t> pickRandomRegions(int numRegions, int platesNum, int seed);
Plates * generatePlates(TriangleMesh * mesh, std::vector<float> points, int N, int P, int seed);
Borders* findCollisions(TriangleMesh * mesh, Map * map);

void assignTriangleValues(TriangleMesh* mesh, Map* map);

std::vector<float> vec3ToXYZ(std::vector<vec3> orig);
std::vector<double> vec2ToDouble(std::vector<vec2> * points);

void draw(int N);

