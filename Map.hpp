#pragma once

#include <vector>
#include "Plates.hpp"
#include "Borders.hpp"
#include "QuadGeometry.hpp"

class Map {
public:
	std::vector<float> region_position;	
	std::vector<float> region_elevation;	
	std::vector<float> region_moisture;
	std::vector<float> region_temperature;

	std::vector<float> triangle_center_position;
	std::vector<float> triangle_center_elevation;
	std::vector<float> triangle_center_moisture;
	std::vector<float> triangle_center_temperature;

	std::vector<int> downflow_sites_for_triangle;
	std::vector<int> river_order_triangles;
	std::vector<float> triangle_flow;
	std::vector<float> sites_flow;

	Plates plates;
	Borders borders;
private:
	void assignTriangleValues();
	void assignRegionElevation();
	void assignDownflow();
	void assignFlow();
};

