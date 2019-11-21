#pragma once

#include <vector>
#include <glm\glm.hpp>

using namespace glm;

class Voronoi {
public:
	std::vector<vec3> points;
	std::vector<vec2> tm;

	Voronoi() {};
	Voronoi(std::vector<float> points, std::vector<float> tm) {

		for (int i = 0; i < points.size() / 3; i++)
			this->points.push_back(vec3(points[i * 3 + 0], points[i * 3 + 1], points[i * 3 + 2]));
		for (int i = 0; i < tm.size() / 2; i++)
			this->tm.push_back(vec2(tm[i * 2 + 0], tm[i * 2 + 1]));
	};
};