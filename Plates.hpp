#pragma once

#include <vector>
#include <glm/glm.hpp>

using namespace glm;

class Plates {
public:
	int seed;

	std::vector<std::size_t>r_plate;
	std::vector<glm::vec3> plate_vec;
	std::vector<std::size_t> region_set;
	std::vector<std::size_t> plates_is_ocean;

	Plates() {};
	Plates(std::vector<std::size_t> r_plate,
		std::vector<glm::vec3> plate_vec,
		std::vector<std::size_t> region_set)
	{
		this->r_plate = r_plate;
		this->plate_vec = plate_vec;
		this->region_set = region_set;
	}


};