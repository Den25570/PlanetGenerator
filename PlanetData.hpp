#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "mesh.h"

using namespace glm;

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
	std::vector<float> r_xyz;
	std::vector<float> t_xyz;
	std::vector<float> r_elevation;
	std::vector<float> t_elevation;
	std::vector<float> r_moisture;
	std::vector<float> t_moisture;
	std::vector<int> t_downflow_s;
	std::vector<float> order_t;
	std::vector<float> t_flow;
	std::vector<float> s_flow;
	Plates plates;
	Borders borders;

};

class QuadGeometry {
public:
	std::vector<size_t> indices; //i
	std::vector<float> points;  //xyz
	std::vector<float> tem_mois; //tm

	void setMesh(TriangleMesh * mesh);
	void setMap(TriangleMesh * mesh, Map * map);
};

class QuadGeometryV {
public:
	std::vector<size_t> indices; //i
	std::vector<vec3> points;  //xyz
	std::vector<vec2> tem_mois; //tm

	QuadGeometryV(QuadGeometry qg)
	{
		indices = qg.indices;
		for (int i = 0; i < qg.points.size()/3; i++)
			points.push_back(vec3(qg.points[i*3+0], qg.points[i * 3 + 1], qg.points[i * 3 + 2]));

		for (int i = 0; i < qg.tem_mois.size() / 2; i++)
			tem_mois.push_back(vec2(qg.tem_mois[i * 2 + 0], qg.tem_mois[i * 2 + 1]));
	}
};