#include "StructToFlat.hpp"

std::vector<float> vec3ToFlat(std::vector<glm::vec3> orig) {
	std::vector<float> res;
	for (auto p : orig)
	{
		res.push_back(p.x);
		res.push_back(p.y);
		res.push_back(p.z);
	}
	return res;
}

std::vector<double> vec2ToDouble(std::vector<glm::vec2> * points) {
	std::vector<double> res;
	for (auto p : *points)
	{
		res.push_back((double)p.x);
		res.push_back((double)p.y);
	}
	return res;
}