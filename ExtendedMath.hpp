#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include  <glm/glm.hpp>

#include <vector>
#include <list>

using namespace glm;

vec3 calcVector(vec3 v0, vec3 v1, float length);

std::vector<vec2>* stereographicProjection(std::vector<vec3> * points);

//Сравнение чисел с плавающей запятой
bool compareF(const float num_1, const float num_2, const float eps = 0.001f);
inline bool compareD(const double num_1, const double num_2, const double eps = 0.001);