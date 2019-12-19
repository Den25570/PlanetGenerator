#pragma once

#include "Map.hpp"
#include "QuadGeometry.hpp"
#include "mesh.h"
#include "Voronoi.hpp"
#include "StructToFlat.hpp"
#include "VAO.hpp"
#include "Object.hpp"

using namespace glm;

class Planet : public Object {
public:
	GLuint texture;

	//structure variables
	Map map;
	TriangleMesh mesh;
	QuadGeometry quadGeometry;
	Voronoi voronoi;

	vec3 axis = vec3(0,0,1);
	float rotating_speed = 0.0f;
	vec3 position;

	VAO noise_VAO;
	VAO voronoi_VAO;
	VAO points_VAO;
	VAO plate_boundaries_VAO;
	VAO plate_vectors_VAO;
	VAO rivers_VAO;

	Planet() { initialized = false; };
	Planet(int region_number, int plates_number, int seed, float water_pecent);
	~Planet();

	void setVAOs(Shader* indexed_triangle_shader, Shader* triangle_shader, Shader* pointShader, Shader* lineShader);
	void draw(int noise, int rivers, glm::mat4 * MVP, glm::mat4 * Model, int outline_strength = 0.0f);

private:
	int seed;
	int plates_number;

	//Map
	void generateMap(float water_percent);
	std::vector<float> generateTriangleCenters();
	std::vector<std::size_t> assignDistanceField(std::vector<std::size_t> seeds_r, std::vector<std::size_t> stop_r);
	void assignTriangleValues();
	void assignRegionElevation(float elevation_noise = 0.4f);
	void assignFlow();
	void assignDownflow();
	void assignRegionMoisure(float water_percent);
	void assignTemperature();

	//QuadGeometry
	void setMesh();
	void setMap();
	//Voronoi
	void generateVoronoiGeometry();
	//Plates
	void generatePlates();
	void generateOceans(float water_percent);
	std::vector<std::size_t> pickRandomRegions();
	//Borders
	Borders findCollisions();

	//drawSet
	std::vector<GLfloat> setDrawVectorPlateBoundaries();
	std::vector<GLfloat> setDrawVectorPlateVectors();
	std::vector<GLfloat> setDrawVectorRivers();
};