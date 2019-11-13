#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "mesh.h"

struct Plates {
	std::vector<int>r_plate;
	std::vector<glm::vec3> plate_vec;
	std::vector<size_t> region_set;
	std::vector<size_t> plates_is_ocean;

	Plates() {};
	Plates(std::vector<int> _r_plate,
		std::vector<glm::vec3> _plate_vec,
		std::vector<size_t> _region_set)
	{
		r_plate = _r_plate;
		plate_vec = _plate_vec;
		region_set = _region_set;
	}
};

struct Borders {
	std::vector<size_t> mountain_r;
	std::vector<size_t>	coastline_r;
	std::vector<size_t>	ocean_r;

	Borders() {};
	Borders(std::vector<size_t> _mountain_r,
		std::vector<size_t>	_coastline_r,
		std::vector<size_t>	_ocean_r)
	{
		mountain_r = _mountain_r;
		coastline_r = _coastline_r;
		ocean_r = _ocean_r;
	}
};

struct Map {
	std::vector<size_t> r_xyz;
	std::vector<size_t> t_xyz;
	std::vector<int> r_elevation;
	std::vector<int> t_elevation;
	std::vector<int> r_moisture;
	std::vector<int> t_moisture;
	std::vector<int> t_downflow_s;
	std::vector<int> order_t;
	std::vector<int> t_flow;
	std::vector<int> s_flow;
	Plates plates;
	Borders borders;
};

class QuadGeometry {
public:
	std::vector<size_t> indices; //i
	std::vector<size_t> points;  //xyz
	std::vector<size_t> tem_mois; //tm

	void setMesh(TriangleMesh * mesh);
	void setMap(TriangleMesh * mesh, Map * map);
};