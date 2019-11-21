#pragma once

#include <vector>
#include "Plates.hpp"
#include "Borders.hpp"
#include "QuadGeometry.hpp"

class Map {
public:
	std::vector<float> r_xyz;
	std::vector<float> t_xyz;
	std::vector<float> r_elevation;
	std::vector<float> t_elevation;
	std::vector<float> r_moisture;
	std::vector<float> t_moisture;
	std::vector<int> t_downflow_s;
	std::vector<int> order_t;
	std::vector<float> t_flow;
	std::vector<float> s_flow;

	Plates plates;
	Borders borders;
private:
	void assignTriangleValues();
	void assignRegionElevation();
	void assignDownflow();
	void assignFlow();
};

