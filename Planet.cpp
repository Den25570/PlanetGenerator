#include "Planet.hpp"
#include "StructToFlat.hpp"
#include "SimplexNoise.h"

#include <iostream>
#include <limits>
#include <deque>

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

using std::vector;
using std::deque;

// Calculate the centroid and push it onto an array 
void pushCentroidOfTriangle(std::vector<float> * out, float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz) {
	vec3 triangle_center = normalize(vec3((ax + bx + cx) / 3.0f, (ay + by + cy) / 3.0f, (az + bz + cz) / 3.0f));
	out->push_back(triangle_center.x);
	out->push_back(triangle_center.y);
	out->push_back(triangle_center.z);
}


std::vector<float> Planet::generateTriangleCenters() {
	std::vector<float> t_xyz;
	std::vector<float> region_position = map.region_position;
	for (int t = 0; t < mesh.numTriangles; t++) {
		int a = mesh.start_site_of_region(3 * t);
		int	b = mesh.start_site_of_region(3 * t + 1);
		int	c = mesh.start_site_of_region(3 * t + 2);
		pushCentroidOfTriangle(&t_xyz,
			region_position[3 * a], region_position[3 * a + 1], region_position[3 * a + 2],
			region_position[3 * b], region_position[3 * b + 1], region_position[3 * b + 2],
			region_position[3 * c], region_position[3 * c + 1], region_position[3 * c + 2]);
	}
	return t_xyz;
}

//generates noise for spherical objects
float getPointElevationNoise(float x, float y, float z) {
	std::vector<float> amplitudes(5);
	const float persistence = 2.0f / 3.0f;
	for (float i = 0; i < amplitudes.size(); i+= 1.0f)
		amplitudes[i] = (powf(persistence, i));

	float sum = 0, sumOfAmplitudes = 0;
	for (int octave = 0; octave < amplitudes.size(); octave++) {
		int frequency = 1 << octave;
		sum += amplitudes[octave] * SimplexNoise::noise(x * frequency, y * frequency, z * frequency);
		sumOfAmplitudes += amplitudes[octave];
	}
	return sum / sumOfAmplitudes;
}


Planet::Planet(int region_number, int plates_number, int seed, float water_percent) {
	this->plates_number = plates_number;
	this->seed = seed;

	std::vector<glm::vec3> fibonacci_sphere = generateFibonacciSphere(region_number, 0.1f);
	this->mesh = generateDelanuaySphere(&fibonacci_sphere);

	fibonacci_sphere.clear();
	fibonacci_sphere.shrink_to_fit();

	generateMap(water_percent);
	initialized = true;
}

Planet::~Planet()
{
	this->mesh.halfedges.clear();
	this->mesh.points.clear();
	this->mesh.t_verticles.clear();
	this->mesh.triangles.clear();

	this->map.borders.coastline_region.clear();
	this->map.borders.mountain_region.clear();
	this->map.borders.ocean_region.clear();

	this->map.plates.plates_is_ocean.clear();
	this->map.plates.plate_vec.clear();
	this->map.plates.region_set.clear();
	this->map.plates.r_plate.clear();

	this->map.river_order_triangles.clear();
	this->map.region_elevation.clear();
	this->map.region_moisture.clear();
	this->map.region_position.clear();
	this->map.sites_flow.clear();
	this->map.downflow_sites_for_triangle.clear();
	this->map.triangle_center_elevation.clear();
	this->map.triangle_flow.clear();
	this->map.triangle_center_moisture.clear();
	this->map.triangle_center_position.clear();

	this->quadGeometry.indices.clear();
	this->quadGeometry.points.clear();
	this->quadGeometry.tem_mois.clear();
}

void Planet::generateMap(float water_percent) {
	//initializing
	srand(seed);
	setMesh();

	map.region_position = vec3ToFlat(mesh.points);
	map.triangle_center_position = generateTriangleCenters();
	map.region_elevation = std::vector<float>(mesh.numRegions);
	map.triangle_center_elevation = std::vector<float>(mesh.numTriangles);
	map.region_moisture = std::vector<float>(mesh.numRegions);
	map.triangle_center_moisture = std::vector<float>(mesh.numTriangles);
	map.downflow_sites_for_triangle = std::vector<int>(mesh.numTriangles);
	map.river_order_triangles = std::vector<int>(mesh.numTriangles);
	map.triangle_flow = std::vector<float>(mesh.numTriangles);
	map.sites_flow = std::vector<float>(mesh.numSides);

    generatePlates();
	generateOceans(water_percent);
	
	assignRegionElevation();
	assignRegionMoisure(water_percent);
	assignTemperature();

	assignTriangleValues();
	assignDownflow();
	assignFlow();

	setMap();
	generateVoronoiGeometry();
}

void Planet::assignTemperature() {
	map.region_temperature = std::vector<float>(mesh.numRegions, 0);
	map.triangle_center_temperature = std::vector<float>(mesh.numTriangles, 0);

	for (unsigned int region = 0; region < mesh.numRegions; region++)
		map.region_temperature[region] = abs(map.region_position[region * 3 + 2]) * abs(map.region_position[region * 3 + 2]) * abs(map.region_position[region * 3 + 2]);

	for (unsigned int region = 0; region < mesh.numRegions; region++)
	{
		auto neighbours = mesh.neighbour_regions(region);
		for (unsigned int neighbour : neighbours)
				map.region_temperature[(rand() % 2) ? region : neighbour] = (map.region_temperature[neighbour] + map.region_temperature[region]) / 2.0f;
	}
	
}

void Planet::assignRegionMoisure(float water_percent) {
	srand(seed);
	for (unsigned int region = 0; region < mesh.numRegions; region++) {
		map.region_moisture[region] = std::min(std::min((abs(map.region_position[3*region+2]) * ((map.plates.r_plate[region] % 20 - 10) / 100.0f + 1.0f)), 0.98f) *  water_percent / 100.0f * 1.5f , 0.98f);
	}
	for (unsigned int region = 0; region < mesh.numRegions; region++)
	{
		auto neighbours = mesh.neighbour_regions(region);
		for (unsigned int neighbour : neighbours)
		if (map.region_elevation[neighbour] >= 0.0f)
			map.region_moisture[(rand() % 2) ? region : neighbour] = (map.region_moisture[neighbour] + map.region_moisture[region]) / 2.0f;
	}
}

void Planet::assignTriangleValues() {
	for (int triangle = 0; triangle < mesh.numTriangles; triangle++) {
		int s0 = 3 * triangle;
		int r1 = mesh.start_site_of_region(s0);
		int	r2 = mesh.start_site_of_region(s0 + 1);
		int	r3 = mesh.start_site_of_region(s0 + 2);
		map.triangle_center_elevation[triangle] = 1.0f / 3.0f * (map.region_elevation[r1] + map.region_elevation[r2] + map.region_elevation[r3]);
		map.triangle_center_moisture[triangle] = 1.0f / 3.0f * (map.region_moisture[r1] + map.region_moisture[r2] + map.region_moisture[r3]);
		map.triangle_center_temperature[triangle] = 1.0f / 3.0f * (map.region_temperature[r1] + map.region_temperature[r2] + map.region_temperature[r3]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    QuadGeometry / Voronoi
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Planet::setMesh() {
	quadGeometry.indices = std::vector<std::size_t>(3 * mesh.numSides);
	quadGeometry.points = std::vector<float>(3 * (mesh.numRegions + mesh.numTriangles));
	quadGeometry.tem_mois = std::vector<float>(3 * (mesh.numRegions + mesh.numTriangles));
}

void Planet::setMap() 
{
	const float V = 0.95f;

	quadGeometry.points = map.region_position;
	for (auto p : map.triangle_center_position)
		quadGeometry.points.push_back(p);

	// TODO: multiply all the region, t points by the elevation, taking V into account

	int p = 0;
	for (int region = 0; region < mesh.numRegions; region++) {
		quadGeometry.tem_mois[p++] = map.region_elevation[region];
		quadGeometry.tem_mois[p++] = map.region_moisture[region];
		quadGeometry.tem_mois[p++] = map.region_temperature[region];
	}
	for (int t = 0; t < mesh.numTriangles; t++) {
		quadGeometry.tem_mois[p++] = map.triangle_center_elevation[t];
		quadGeometry.tem_mois[p++] = map.triangle_center_moisture[t];
		quadGeometry.tem_mois[p++] = map.triangle_center_temperature[t];
	}

	int i = 0;
	int count_valley = 0;
	int count_ridge = 0;

	for (int s = 0; s < mesh.numSides; s++) {
		int opposite_s = mesh.s_opposite_s(s),
			r1 = mesh.start_site_of_region(s),
			r2 = mesh.start_site_of_region(opposite_s),
			t1 = mesh.inner_triangle(s),
			t2 = mesh.inner_triangle(opposite_s);

		int coast = map.region_elevation[r1] < 0.0 || map.region_elevation[r2] < 0.0;
		if (coast || map.sites_flow[s] > 0 || map.sites_flow[opposite_s] > 0) {
			// Valley
			quadGeometry.indices[i++] = r1;
			quadGeometry.indices[i++] = mesh.numRegions + t2;
			quadGeometry.indices[i++] = mesh.numRegions + t1;
			count_valley++;
		}
		else {
			// Ridge
			quadGeometry.indices[i++] = r1;
			quadGeometry.indices[i++] = r2;
			quadGeometry.indices[i++] = mesh.numRegions + t1;
			count_ridge++;
		}
	}
}

void Planet::generateVoronoiGeometry() {
	std::vector<float> xyz;
	std::vector<float> tm;

	for (int s = 0; s < mesh.numSides; s++) {
		int inner_t = mesh.inner_triangle(s);
		int outer_t = mesh.outer_triangle(s);
		int	begin_r = mesh.start_site_of_region(s);

		xyz.push_back(map.triangle_center_position[3 * inner_t]);
		xyz.push_back(map.triangle_center_position[3 * inner_t + 1]);
		xyz.push_back(map.triangle_center_position[3 * inner_t + 2]);

		xyz.push_back(map.triangle_center_position[3 * outer_t]);
		xyz.push_back(map.triangle_center_position[3 * outer_t + 1]);
		xyz.push_back(map.triangle_center_position[3 * outer_t + 2]);

		xyz.push_back(map.region_position[3 * begin_r]);
		xyz.push_back(map.region_position[3 * begin_r + 1]);
		xyz.push_back(map.region_position[3 * begin_r + 2]);
				
		for (int i = 0; i < 3; i++)
		{
			tm.push_back(map.region_elevation[begin_r]);
			tm.push_back(map.region_moisture[begin_r]);
			tm.push_back(map.region_temperature[begin_r]);
		}
	}
	voronoi = Voronoi(xyz, tm);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    Plates 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::size_t> Planet::pickRandomRegions() {
	srand(seed);
	std::vector<std::size_t> region_indices;
	while (region_indices.size() < plates_number && region_indices.size() < mesh.numRegions) {
		region_indices.push_back(rand() % mesh.numRegions);
	}
	return region_indices;
}

void Planet::generatePlates() {
	std::vector<std::size_t> plates_regions_indices = std::vector<std::size_t>(mesh.numRegions, std::numeric_limits<std::size_t>::max());

	auto plates_center_indices = pickRandomRegions();

	
	deque<GLuint> queue;
	for (int i = 0; i < plates_center_indices.size(); i++)
		queue.push_back(plates_center_indices[i]);

	for (auto region : queue) { plates_regions_indices[region] = region; }
	//Random assignment region to plate

	int undefined_regions = plates_regions_indices.size() - plates_center_indices.size();
	
		while (!queue.empty())
		{
			int current_region = queue.front();
			queue.pop_front();
			std::vector<std::size_t> neighbor_regions = mesh.neighbour_regions(current_region);
			for (auto neighbor_region : neighbor_regions) {
				if (plates_regions_indices[neighbor_region] == std::numeric_limits<std::size_t>::max()) {
					plates_regions_indices[neighbor_region] = plates_regions_indices[current_region];
					queue.push_back(neighbor_region);
					undefined_regions--;
				}
			}
		}

	while (undefined_regions != 0)
	{
		for (int i = 0; i < plates_regions_indices.size(); i++)
		{
			if (plates_regions_indices[i] == std::numeric_limits<std::size_t>::max()) {

				std::vector<std::size_t> neighbor_regions = mesh.neighbour_regions(i);
				for (auto neighbor_region : neighbor_regions) {
					if (plates_regions_indices[neighbor_region] != std::numeric_limits<std::size_t>::max()) {
						plates_regions_indices[i] = plates_regions_indices[neighbor_region];
						undefined_regions--;
						break;
					}
				}
			}
		}
	}

	// Assign a random movement vector for each plate
	std::vector<vec3> plates_move_direction = std::vector<vec3>(mesh.numRegions);
	for (auto center_region : plates_center_indices) {
		auto neighbor_region = mesh.neighbour_regions(center_region)[0];
		vec3 p0 = vec3(map.region_position[3 * center_region], map.region_position[3 * center_region + 1], map.region_position[3 * center_region + 2]);
		vec3 p1 = vec3(map.region_position[3 * neighbor_region], map.region_position[3 * neighbor_region + 1], map.region_position[3 * neighbor_region + 2]);
		plates_move_direction[center_region] = normalize(p1 - p0);
	}

	map.plates = Plates(plates_regions_indices, plates_move_direction, plates_center_indices);
}

void Planet::generateOceans(float water_percent) {
	srand(seed);
	for (auto region : map.plates.region_set) {
		if (rand() % (100) < water_percent) {
			map.plates.plates_is_ocean.push_back(region);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    Elevation/Rivers 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Planet::assignRegionElevation(float elevation_noise) {
	const float epsilon = 1e-3f;

	Borders borders = findCollisions();

	for (int region = 0; region < mesh.numRegions; region++) {
		if (map.plates.r_plate[region] == region) {
			(std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), region) != map.plates.plates_is_ocean.end() ?
				borders.ocean_region : borders.coastline_region).push_back(region);
		}
	}

	std::vector<std::size_t> stop_regions;
	for (auto region : borders.mountain_region) { stop_regions.push_back(region); }
	for (auto region : borders.coastline_region) { stop_regions.push_back(region); }
	for (auto region : borders.ocean_region) { stop_regions.push_back(region); }

	std::vector<std::size_t> r_distance_a = assignDistanceField(borders.mountain_region, borders.ocean_region);
	std::vector<std::size_t> r_distance_b = assignDistanceField(borders.ocean_region, borders.coastline_region);
	std::vector<std::size_t> r_distance_c = assignDistanceField(borders.coastline_region, stop_regions);

	for (auto region = 0; region < mesh.numRegions; region++) {
		float a = pow(r_distance_a[region] + epsilon, 1);
		float b = r_distance_b[region] + epsilon;
		float c = r_distance_c[region] + epsilon;
		if (a == std::numeric_limits<std::size_t>::max() + epsilon && b == std::numeric_limits<std::size_t>::max() + epsilon) {
			map.region_elevation[region] = 0.1f;
		}
		else {
			map.region_elevation[region] = (1.0f / a - 1.0f / b) / (1.0f / a + 1.0f / b + 1.0f / c);
		}
		map.region_elevation[region] += elevation_noise * getPointElevationNoise(map.region_position[3 * region], map.region_position[3 * region + 1], map.region_position[3 * region + 2]);
	}
}

void Planet::assignDownflow() {
	/* Use a priority queue, starting with the ocean triangles and
	 * moving upwards using elevation as the priority, to visit all
	 * the land triangles */
	struct priorityQD {
		int value;
		float comp;

		priorityQD() {};
		priorityQD(int value, float comp) {
			this->value = value;
			this->comp = comp;
		}
	};

	std::list<priorityQD> queue;
	int queue_in = 0;
	map.downflow_sites_for_triangle = std::vector<int>(mesh.numTriangles, -999);
	/* Part 1: ocean triangles get downslope assigned to the lowest neighbor */
	for (int t = 0; t < mesh.numTriangles; t++) {
		if (map.triangle_center_elevation[t] < 0) {

			int best_s = -1;
			float best_e = map.triangle_center_elevation[t];
			for (int j = 0; j < 3; j++) {
				int s = 3 * t + j;
				float e = map.triangle_center_elevation[mesh.outer_triangle(s)];
				if (e < best_e) {
					best_e = e;
					best_s = s;
				}
			}
			map.river_order_triangles[queue_in++] = t;
			map.downflow_sites_for_triangle[t] = best_s;

			queue.push_back(priorityQD(t, map.triangle_center_elevation[t]));
		}
	}
	/* Part 2: land triangles get visited in elevation priority */
	for (int queue_out = 0; queue_out < mesh.numTriangles; queue_out++) {
		int current_triangle = queue.front().value;
		queue.pop_front();
		for (int j = 0; j < 3; j++) {
			int s = 3 * current_triangle + j;
			int neighbor_t = mesh.outer_triangle(s); // uphill from current_t
			if (map.downflow_sites_for_triangle[neighbor_t] == -999 && map.triangle_center_elevation[neighbor_t] >= 0.0) {
				map.downflow_sites_for_triangle[neighbor_t] = mesh.s_opposite_s(s);
				map.river_order_triangles[queue_in++] = neighbor_t;

				queue.push_back(priorityQD(neighbor_t, map.triangle_center_elevation[neighbor_t]));
			}
		}
	}
}


void Planet::assignFlow() {
	map.sites_flow = std::vector<float>(map.sites_flow.size(), 0);
	for (int t = 0; t < mesh.numTriangles; t++) {
		if ((map.triangle_center_elevation[t] >= 0.0f) && (rand() % 10)) {
				map.triangle_flow[t] = 0.5f * map.triangle_center_moisture[t] * map.triangle_center_moisture[t];
		}
		else {
			map.triangle_flow[t] = 0;
		}
	}
	for (int i = map.river_order_triangles.size() - 1; i >= 0; i--) {
		int tributary_t = map.river_order_triangles[i];
		int flow_sites = map.downflow_sites_for_triangle[tributary_t];
		if (flow_sites == -1) continue;
		int trunk_t = (mesh.halfedges[flow_sites] / 3) | 0;
		if (flow_sites >= 0) {
			map.triangle_flow[trunk_t] += map.triangle_flow[tributary_t];
			map.sites_flow[flow_sites] += map.triangle_flow[tributary_t];
			if (map.triangle_center_elevation[trunk_t] > map.triangle_center_elevation[tributary_t]) {
				map.triangle_center_elevation[trunk_t] = map.triangle_center_elevation[tributary_t];
			}
		}
	}
}

/*Distance from any point in seeds_r to all other points, but
* don't go past any point in stop_r */
std::vector<std::size_t> Planet::assignDistanceField(std::vector<std::size_t> seeds_r, std::vector<std::size_t> stop_r) {
	srand(seed);
	std::vector<std::size_t> region_distance = std::vector<std::size_t>(mesh.numRegions, std::numeric_limits<std::size_t>::max());

	std::vector<std::size_t> queue;
	for (auto region : seeds_r) {
		queue.push_back(region);
		region_distance[region] = 0;
	}

	std::vector<std::size_t> neighbors;
	for (int queue_out = 0; queue_out < mesh.numRegions; queue_out++) {
		if (queue.size() == queue_out)
			return region_distance;

		int pos = queue_out + rand() % (queue.size() - queue_out);
		int current_r = queue[pos];
		queue[pos] = queue[queue_out];
		neighbors = mesh.neighbour_regions(current_r);
		for (auto neighbor : neighbors) {
			if (region_distance[neighbor] == std::numeric_limits<std::size_t>::max() && !(std::find(stop_r.begin(), stop_r.end(), neighbor) != stop_r.end())) {
				region_distance[neighbor] = region_distance[current_r] + 1;
				queue.push_back(neighbor);
			}
		}
	}
	return region_distance;
}

Borders Planet::findCollisions() {
	const float deltaTime = 1e-2f; // simulate movement
	const float COLLISION_THRESHOLD = 0.75f;
	std::vector<std::size_t> mountain_region;
	std::vector<std::size_t> coastline_region;
	std::vector<std::size_t> ocean_region;
	std::vector<std::size_t> neighbours;
	for (int current_region = 0; current_region < mesh.numRegions; current_region++) {
		float bestCompression = std::numeric_limits<float>::max();
		int best_r = -1;
		neighbours = mesh.neighbour_regions(current_region);
		for (auto neighbor : neighbours) {
			if (map.plates.r_plate[current_region] != map.plates.r_plate[neighbor]) {
				vec3 current_pos = vec3(map.region_position[3 * current_region], map.region_position[3 * current_region + 1], map.region_position[3 * current_region + 2]);
				vec3 neighbor_pos = vec3(map.region_position[3 * neighbor], map.region_position[3 * neighbor + 1], map.region_position[3 * neighbor + 2]);
				float distanceBefore = distance(current_pos, neighbor_pos);
				float distanceAfter = distance(current_pos + map.plates.plate_vec[map.plates.r_plate[current_region]] * deltaTime,
					neighbor_pos + map.plates.plate_vec[map.plates.r_plate[neighbor]] * deltaTime);

				float compression = distanceBefore - distanceAfter;
				if (compression < bestCompression) {
					best_r = neighbor;
					bestCompression = compression;
				}
			}
		}
		if (best_r != -1) {
			bool collided = bestCompression > COLLISION_THRESHOLD * deltaTime;
			if (std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), current_region) != map.plates.plates_is_ocean.end() &&
				std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), best_r) != map.plates.plates_is_ocean.end()) {
				(collided ? coastline_region : ocean_region).push_back(current_region);
			}
			else if (std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), current_region) == map.plates.plates_is_ocean.end() &&
				std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), best_r) == map.plates.plates_is_ocean.end()) {
				if (collided) mountain_region.push_back(current_region);
			}
			else {
				(collided ? mountain_region : coastline_region).push_back(current_region);
			}
		}
	}
	return Borders(mountain_region, coastline_region, ocean_region);
}

void Planet::setVAOs(Shader* indexed_triangle_shader, Shader* triangle_shader, Shader* pointShader, Shader* lineShader)
{
	std::vector<GLfloat> vertices; std::vector<GLuint> indices;
	quadToFlat(vertices, indices, &quadGeometry);
	noise_VAO = VAO(indexed_triangle_shader, std::vector<int> {3, 3}, &vertices, &indices);
	
	std::vector<GLfloat> vertices_voronoi;
	voronoiToFlat(vertices_voronoi, &voronoi);
	voronoi_VAO = VAO(triangle_shader, std::vector<int> {3, 3}, &vertices_voronoi);

	points_VAO = VAO(pointShader, std::vector<int> {3}, &map.region_position);
	plate_boundaries_VAO = VAO(lineShader, std::vector<int> {3, 4}, &setDrawVectorPlateBoundaries());

	std::vector<GLfloat> plate_vectors = setDrawVectorPlateVectors();
	if (!plate_vectors.empty()) plate_vectors_VAO = VAO(lineShader, std::vector<int> {3, 4}, &plate_vectors);

	std::vector<GLfloat> rivers = setDrawVectorRivers();
	if (!rivers.empty()) rivers_VAO = VAO(lineShader, std::vector<int> {3, 4}, &rivers);

	//Освобождение ресурсов
	vertices.clear();
	indices.clear();
	vertices_voronoi.clear();
}

void drawLines(VAO &VAO, glm::mat4 &MVP)
{
	VAO.shader->Use();
	VAO.use(GL_FRONT_AND_BACK, GL_LINE);
	glUniformMatrix4fv(glGetUniformLocation(VAO.shader->Program, "u_projection"), 1, GL_FALSE, &MVP[0][0]);
	glUniform4f(glGetUniformLocation(VAO.shader->Program, "u_add_rgba"), 0, 0, 0, 0);
	glUniform4f(glGetUniformLocation(VAO.shader->Program, "u_multiply_rgba"), 1, 1, 1, 1);
	VAO.draw(GL_LINES);
}

void Planet::draw(int noise, int rivers, glm::mat4 * MVP, glm::mat4 * Model, int outline_strength)
{
	if (noise) {
		noise_VAO.shader->Use();
		noise_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform2f(glGetUniformLocation(noise_VAO.shader->Program, "u_light_angle"), cosf(M_PI / 3.0f), sinf(M_PI / 3.0f));
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_inverse_texture_size"), 1.0f / 2048.0f);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_d"), 60.0f);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_c"), 0.15f);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_slope"), 6.0f);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_flat"), 2.5f);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "u_outline_strength"), outline_strength);
		glUniform3f(glGetUniformLocation(noise_VAO.shader->Program, "light_color"), 1, 1, 1);
		glUniform3f(glGetUniformLocation(noise_VAO.shader->Program, "light_pos"), 20, 0, 0);
		glUniform1f(glGetUniformLocation(noise_VAO.shader->Program, "ambitient_strength"), 0.02f);
		glUniformMatrix4fv(glGetUniformLocation(noise_VAO.shader->Program, "model"), 1, GL_FALSE, &(*Model)[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(noise_VAO.shader->Program, "u_projection"), 1, GL_FALSE, &(*MVP)[0][0]);
		noise_VAO.draw(GL_TRIANGLES);
	}
	else if (!noise) {
		voronoi_VAO.shader->Use();
		voronoi_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform3f(glGetUniformLocation(voronoi_VAO.shader->Program, "light_color"), 1, 1, 1);
		glUniform3f(glGetUniformLocation(voronoi_VAO.shader->Program, "light_pos"), 20, 0, 0);
		glUniform1f(glGetUniformLocation(voronoi_VAO.shader->Program, "ambitient_strength"), 0.02f);
		glUniformMatrix4fv(glGetUniformLocation(voronoi_VAO.shader->Program, "model"), 1, GL_FALSE, &(*Model)[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(voronoi_VAO.shader->Program, "u_projection"), 1, GL_FALSE, &(*MVP)[0][0]);
		voronoi_VAO.draw(GL_TRIANGLES);
	}
	if (rivers && rivers_VAO.initialized()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		glBlendColor(0, 0, 0, 0);

		rivers_VAO.shader->Use();
		rivers_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
		glUniform3f(glGetUniformLocation(rivers_VAO.shader->Program, "light_color"), 1, 1, 1);
		glUniform3f(glGetUniformLocation(rivers_VAO.shader->Program, "light_pos"), 20, 0, 0);
		glUniform1f(glGetUniformLocation(rivers_VAO.shader->Program, "ambitient_strength"), 0.02f);
		glUniformMatrix4fv(glGetUniformLocation(rivers_VAO.shader->Program, "model"), 1, GL_FALSE, &(*Model)[0][0]);
		glUniform4f(glGetUniformLocation(rivers_VAO.shader->Program, "u_multiply_rgba"), 1, 1, 1, 1);
		glUniform4f(glGetUniformLocation(rivers_VAO.shader->Program, "u_add_rgba"), 0, 0, 0, 0);
		glUniformMatrix4fv(glGetUniformLocation(rivers_VAO.shader->Program, "u_projection"), 1, GL_FALSE, &(*MVP)[0][0]);
		rivers_VAO.draw(GL_LINES);

		glDisable(GL_BLEND);
	}
}

std::vector<GLfloat> Planet::setDrawVectorPlateBoundaries() {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int s = 0; s < mesh.numSides; s++) {
		int begin_r = mesh.start_site_of_region(s);
		int	end_r = mesh.s_end_r(s);
		if (map.plates.r_plate[begin_r] != map.plates.r_plate[end_r]) {
			int inner_t = mesh.inner_triangle(s);
			int	outer_t = mesh.outer_triangle(s);

			line_xyz.push_back(vec3(map.triangle_center_position[3 * inner_t + 0], map.triangle_center_position[3 * inner_t + 1], map.triangle_center_position[3 * inner_t + 2]));
			line_xyz.push_back(vec3(map.triangle_center_position[3 * outer_t + 0], map.triangle_center_position[3 * outer_t + 1], map.triangle_center_position[3 * outer_t + 2]));

			line_rgba.push_back(vec4(1, 1, 1, 1));
			line_rgba.push_back(vec4(1, 1, 1, 1));
		}
	}

	std::vector<GLfloat> plateBoundariesDrawvector = std::vector<GLfloat>(line_xyz.size() * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		plateBoundariesDrawvector[i * 7 + 0] = line_xyz[i].x;
		plateBoundariesDrawvector[i * 7 + 1] = line_xyz[i].y;
		plateBoundariesDrawvector[i * 7 + 2] = line_xyz[i].z;
		plateBoundariesDrawvector[i * 7 + 3] = line_rgba[i].r;
		plateBoundariesDrawvector[i * 7 + 4] = line_rgba[i].g;
		plateBoundariesDrawvector[i * 7 + 5] = line_rgba[i].b;
		plateBoundariesDrawvector[i * 7 + 6] = line_rgba[i].a;
	}
	return plateBoundariesDrawvector;
}

std::vector<GLfloat> Planet::setDrawVectorPlateVectors() {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int region = 0; region < mesh.numRegions; region++) {
		line_xyz.push_back(vec3(map.region_position[region * 3 + 0], map.region_position[region * 3 + 1], map.region_position[region * 3 + 2]));
		line_rgba.push_back(vec4(1, 1, 1, 1));
		line_xyz.push_back(vec3(map.region_position[region * 3 + 0], map.region_position[region * 3 + 1], map.region_position[region * 3 + 2]) + map.plates.plate_vec[map.plates.r_plate[region]] * (2 / sqrtf(1000)));
		line_rgba.push_back(vec4(1, 0, 0, 0));
	}

	std::vector<GLfloat> plateVectorsDrawvector = std::vector<GLfloat>(line_xyz.size() * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		plateVectorsDrawvector[i * 7 + 0] = line_xyz[i].x;
		plateVectorsDrawvector[i * 7 + 1] = line_xyz[i].y;
		plateVectorsDrawvector[i * 7 + 2] = line_xyz[i].z;
		plateVectorsDrawvector[i * 7 + 3] = line_rgba[i].r;
		plateVectorsDrawvector[i * 7 + 4] = line_rgba[i].g;
		plateVectorsDrawvector[i * 7 + 5] = line_rgba[i].b;
		plateVectorsDrawvector[i * 7 + 6] = line_rgba[i].a;
	}
	return plateVectorsDrawvector;
}

std::vector<GLfloat> Planet::setDrawVectorRivers() {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int s = 0; s < mesh.numSides; s++) {
		if (map.sites_flow[s] > 1) {
			float flow = 0.1f * sqrtf(map.sites_flow[s]);
			int inner_triangle = mesh.inner_triangle(s);
			int	outer_triangle = mesh.outer_triangle(s);
			line_xyz.push_back(vec3(map.triangle_center_position[3 * inner_triangle], map.triangle_center_position[3 * inner_triangle + 1], map.triangle_center_position[3 * inner_triangle + 2]));
			line_xyz.push_back(vec3(map.triangle_center_position[3 * outer_triangle], map.triangle_center_position[3 * outer_triangle + 1], map.triangle_center_position[3 * outer_triangle + 2]));
			if (flow > 1) flow = 1;
			vec4 rgba_premultiplied = vec4(0.2f * 0.5, 0.5f * 0.5, 0.7f * 0.5, 0.8);
			line_rgba.push_back(rgba_premultiplied);
			line_rgba.push_back(rgba_premultiplied);
		}
	}

	std::vector<GLfloat> riverDrawvector = std::vector<GLfloat>(line_xyz.size() * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		riverDrawvector[i * 7 + 0] = line_xyz[i].x;
		riverDrawvector[i * 7 + 1] = line_xyz[i].y;
		riverDrawvector[i * 7 + 2] = line_xyz[i].z;
		riverDrawvector[i * 7 + 3] = line_rgba[i].r;
		riverDrawvector[i * 7 + 4] = line_rgba[i].g;
		riverDrawvector[i * 7 + 5] = line_rgba[i].b;
		riverDrawvector[i * 7 + 6] = line_rgba[i].a;
	}
	return riverDrawvector;
}
