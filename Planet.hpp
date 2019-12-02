#pragma once

#include "Map.hpp"
#include "QuadGeometry.hpp"
#include "mesh.h"
#include "Voronoi.hpp"

using namespace glm;

class Planet {
public:
	Map map;
	TriangleMesh mesh;
	QuadGeometry quadGeometry;
	QuadGeometryV quadGeometryV;
	Voronoi voronoi;

	Planet() { };
	Planet(TriangleMesh mesh, int seed, int region_number, int plates_number);

private:
	int seed;
	int region_number;
	int plates_number;

	//Map
	void generateMap();
	std::vector<float> generateTriangleCenters();
	std::vector<std::size_t> assignDistanceField(std::vector<std::size_t> seeds_r, std::vector<std::size_t> stop_r);
	void assignTriangleValues();
	void assignRegionElevation();
	void assignFlow();
	void assignDownflow();

	//QuadGeometry
	void setMesh();
	void setMap();

	//Voronoi
	void generateVoronoiGeometry();

	//Plates
	void generatePlates();
	void generateOceans();
	std::vector<std::size_t> pickRandomRegions();

	//Borders
	Borders findCollisions();
};