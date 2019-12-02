#include "ExtendedMath.hpp"

vec3 calcVector(vec3 v0, vec3 v1, float length)
{
	return  v0 - normalize(v0 - v1)*length;
}

std::vector<vec2>* stereographicProjection(std::vector<vec3> * points)
{
	auto new_points = new std::vector<vec2>();
	for (auto p : *points)
	{
		new_points->push_back(vec2(p.x / (1.0f - p.z), p.y / (1.0f - p.z)));
	}
	return new_points;
}

bool compareF(const float num_1, const float num_2, const float eps)
{
	return abs(num_1 - num_2) < eps;
}

inline bool compareD(const double num_1, const double num_2, const double eps)
{
	return abs(num_1 - num_2) < eps;
}