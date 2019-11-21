#pragma once

#include <vector>
#include <glm/glm.hpp>

std::vector<float> vec3ToFlat(std::vector<glm::vec3> orig);

std::vector<double> vec2ToDouble(std::vector<glm::vec2> * points);
