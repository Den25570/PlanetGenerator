#include "Planet.hpp"
#include "StructToFlat.hpp"
#include "SimplexNoise.h"

#include <iostream>
#include <limits>

/* Calculate the centroid and push it onto an array */
void pushCentroidOfTriangle(std::vector<float> * out, float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz) {
	// TODO: renormalize to radius 1
	out->push_back((ax + bx + cx) / 3.0f);
	out->push_back((ay + by + cy) / 3.0f);
	out->push_back((az + bz + cz) / 3.0f);
}


std::vector<float> Planet::generateTriangleCenters() {
	std::vector<float> t_xyz;
	std::vector<float> r_xyz = map.r_xyz;
	for (int t = 0; t < mesh.numTriangles; t++) {
		int a = mesh.s_begin_r(3 * t);
		int	b = mesh.s_begin_r(3 * t + 1);
		int	c = mesh.s_begin_r(3 * t + 2);
		pushCentroidOfTriangle(&t_xyz,
			r_xyz[3 * a], r_xyz[3 * a + 1], r_xyz[3 * a + 2],
			r_xyz[3 * b], r_xyz[3 * b + 1], r_xyz[3 * b + 2],
			r_xyz[3 * c], r_xyz[3 * c + 1], r_xyz[3 * c + 2]);
	}
	return t_xyz;
}

float fbm_noise(float nx, float ny, float nz) {
	std::vector<float> amplitudes(5);
	const float persistence = 2.0f / 3.0f;
	for (int i = 0; i < amplitudes.size(); i++)
		amplitudes[i] = (powf(persistence, i*1.0f));

	float sum = 0, sumOfAmplitudes = 0;
	for (int octave = 0; octave < amplitudes.size(); octave++) {
		auto frequency = 1 << octave;
		sum += amplitudes[octave] * SimplexNoise::noise(nx * frequency, ny * frequency, nz * frequency);
		sumOfAmplitudes += amplitudes[octave];
	}
	return sum / sumOfAmplitudes;
}


Planet::Planet(TriangleMesh mesh, int seed, int region_number, int plates_number) {
	this->mesh = mesh;
	this->seed = seed;
	this->region_number = region_number;
	this->plates_number = plates_number;

	generateMap();
}

void Planet::generateMap() {
	//initializing
	srand(seed);
	setMesh();

	map.r_xyz = vec3ToFlat(mesh.points);
	map.t_xyz = generateTriangleCenters();
	map.r_elevation = std::vector<float>(mesh.numRegions);
	map.t_elevation = std::vector<float>(mesh.numTriangles);
	map.r_moisture = std::vector<float>(mesh.numRegions);
	map.t_moisture = std::vector<float>(mesh.numTriangles);
	map.t_downflow_s = std::vector<int>(mesh.numTriangles);
	map.order_t = std::vector<int>(mesh.numTriangles);
	map.t_flow = std::vector<float>(mesh.numTriangles);
	map.s_flow = std::vector<float>(mesh.numSides);

    generatePlates();
	generateOceans();
	
	assignRegionElevation();
	// TODO: assign region moisture in a better way!
	for (int r = 0; r < mesh.numRegions; r++) {
		map.r_moisture[r] = 0.98;//(map.plates.r_plate[r] % 10) / 10.0f;
	}

	assignTriangleValues();
	assignDownflow();
	assignFlow();

	setMap();
	generateVoronoiGeometry();

	quadGeometryV = QuadGeometryV(quadGeometry);
}

void Planet::assignTriangleValues() {
	for (int t = 0; t < mesh.numTriangles; t++) {
		int s0 = 3 * t;
		int r1 = mesh.s_begin_r(s0);
		int	r2 = mesh.s_begin_r(s0 + 1);
		int	r3 = mesh.s_begin_r(s0 + 2);
		map.t_elevation[t] = 1.0f / 3.0f * (map.r_elevation[r1] + map.r_elevation[r2] + map.r_elevation[r3]);
		map.t_moisture[t] = 1.0f / 3.0f * (map.r_moisture[r1] + map.r_moisture[r2] + map.r_moisture[r3]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    QuadGeometry / Voronoi
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Planet::setMesh() {
	quadGeometry.indices = std::vector<std::size_t>(3 * mesh.numSides);
	quadGeometry.points = std::vector<float>(3 * (mesh.numRegions + mesh.numTriangles));
	quadGeometry.tem_mois = std::vector<float>(2 * (mesh.numRegions + mesh.numTriangles));
}

void Planet::setMap() {
	const float V = 0.95f;

	quadGeometry.points = map.r_xyz;
	for (auto p : map.t_xyz)
		quadGeometry.points.push_back(p);

	// TODO: multiply all the r, t points by the elevation, taking V into account

	int p = 0;
	for (int r = 0; r < mesh.numRegions; r++) {
		quadGeometry.tem_mois[p++] = map.r_elevation[r];
		quadGeometry.tem_mois[p++] = map.r_moisture[r];
	}
	for (int t = 0; t < mesh.numTriangles; t++) {
		quadGeometry.tem_mois[p++] = map.t_elevation[t];
		quadGeometry.tem_mois[p++] = map.t_moisture[t];
	}

	int i = 0;
	int count_valley = 0;
	int count_ridge = 0;

	for (int s = 0; s < mesh.numSides; s++) {
		int opposite_s = mesh.s_opposite_s(s),
			r1 = mesh.s_begin_r(s),
			r2 = mesh.s_begin_r(opposite_s),
			t1 = mesh.s_inner_t(s),
			t2 = mesh.s_inner_t(opposite_s);

		// Each quadrilateral is turned into two triangles, so each
		// half-edge gets turned into one. There are two ways to fold
		// a quadrilateral. This is usually a nuisance but in this
		// case it's a feature. See the explanation here
		// https://www.redblobgames.com/x/1725-procedural-elevation/#rendering
		int coast = map.r_elevation[r1] < 0.0 || map.r_elevation[r2] < 0.0;
		if (coast || map.s_flow[s] > 0 || map.s_flow[opposite_s] > 0) {
			// It's a coastal or river edge, forming a valley
			quadGeometry.indices[i++] = r1;
			quadGeometry.indices[i++] = mesh.numRegions + t2;
			quadGeometry.indices[i++] = mesh.numRegions + t1;
			count_valley++;
		}
		else {
			// It's a ridge
			quadGeometry.indices[i++] = r1;
			quadGeometry.indices[i++] = r2;
			quadGeometry.indices[i++] = mesh.numRegions + t1;
			count_ridge++;
		}
	}
	std::cout << count_valley << std::endl;
	std::cout << count_ridge << std::endl;
}

void Planet::generateVoronoiGeometry() {
	std::vector<float> xyz;
	std::vector<float> tm;

	for (int s = 0; s < mesh.numSides; s++) {
		int inner_t = mesh.s_inner_t(s);
		int outer_t = mesh.s_outer_t(s);
		int	begin_r = mesh.s_begin_r(s);

		xyz.push_back(map.t_xyz[3 * inner_t]);
		xyz.push_back(map.t_xyz[3 * inner_t + 1]);
		xyz.push_back(map.t_xyz[3 * inner_t + 2]);

		xyz.push_back(map.t_xyz[3 * outer_t]);
		xyz.push_back(map.t_xyz[3 * outer_t + 1]);
		xyz.push_back(map.t_xyz[3 * outer_t + 2]);

		xyz.push_back(map.r_xyz[3 * begin_r]);
		xyz.push_back(map.r_xyz[3 * begin_r + 1]);
		xyz.push_back(map.r_xyz[3 * begin_r + 2]);
				
		for (int i = 0; i < 3; i++)
		{
			tm.push_back(map.r_elevation[begin_r]);
			tm.push_back(map.r_moisture[begin_r]);
		}
	}
	voronoi = Voronoi(xyz, tm);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    Plates 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::size_t> Planet::pickRandomRegions() {
	std::vector<std::size_t> region_indices;
	while (region_indices.size() < plates_number && region_indices.size() < region_number) {
		region_indices.push_back(rand() % region_number);
	}
	return region_indices;
}

void Planet::generatePlates() {
	std::vector<std::size_t> plates_regions_indices = std::vector<std::size_t>(mesh.numRegions, std::numeric_limits<std::size_t>::max());

	auto plates_center_indices = pickRandomRegions();
	auto queue = plates_center_indices;

	for (auto r : queue) { plates_regions_indices[r] = r; }
	std::vector<std::size_t> neighbor_regions;

	//Random assignment region to plate
	for (int queue_out = 0; queue_out < mesh.numRegions; queue_out++) {
		int pos = queue_out + rand() % (queue.size() - queue_out);
		int current_region = queue[pos];
		queue[pos] = queue[queue_out];

		neighbor_regions = mesh.r_circulate_r(current_region);
		for (auto neighbor_region : neighbor_regions) {
			if (plates_regions_indices[neighbor_region] == std::numeric_limits<std::size_t>::max()) {
				plates_regions_indices[neighbor_region] = plates_regions_indices[current_region];
				queue.push_back(neighbor_region);
			}
		}
	}

	// Assign a random movement vector for each plate
	std::vector<vec3> plates_move_direction = std::vector<vec3>(region_number);
	for (auto center_region : plates_center_indices) {
		auto neighbor_region = mesh.r_circulate_r(center_region)[0];
		vec3 p0 = vec3(map.r_xyz[3 * center_region], map.r_xyz[3 * center_region + 1], map.r_xyz[3 * center_region + 2]);
		vec3 p1 = vec3(map.r_xyz[3 * neighbor_region], map.r_xyz[3 * neighbor_region + 1], map.r_xyz[3 * neighbor_region + 2]);
		plates_move_direction[center_region] = normalize(p1 - p0);
	}

	map.plates = Plates(plates_regions_indices, plates_move_direction, plates_center_indices);
}

void Planet::generateOceans() {
	for (auto region : map.plates.region_set) {
		if (rand() % (10) < 7) {
			map.plates.plates_is_ocean.push_back(region);
			// TODO: either make tiny plates non-ocean, or make sure tiny plates don't create seeds for rivers
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //                                    Elevation/Rivers 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Planet::assignRegionElevation() {
	const float epsilon = 1e-3f;

	auto borders = findCollisions();

	for (int r = 0; r < mesh.numRegions; r++) {
		if (map.plates.r_plate[r] == r) {
			(std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), r) != map.plates.plates_is_ocean.end() ?
				borders.ocean_r : borders.coastline_r).push_back(r);
		}
	}

	std::vector<std::size_t> stop_r;
	for (auto r : borders.mountain_r) { stop_r.push_back(r); }
	for (auto r : borders.coastline_r) { stop_r.push_back(r); }
	for (auto r : borders.ocean_r) { stop_r.push_back(r); }

	std::cout << "seeds mountain/coastline/ocean:" << borders.mountain_r.size() << " " << borders.coastline_r.size() << " " << borders.ocean_r.size() << " " << "plate_is_ocean: " << map.plates.plates_is_ocean.size() << std::endl;
	std::vector<std::size_t> r_distance_a = assignDistanceField(borders.mountain_r, borders.ocean_r);
	std::vector<std::size_t> r_distance_b = assignDistanceField(borders.ocean_r, borders.coastline_r);
	std::vector<std::size_t> r_distance_c = assignDistanceField(borders.coastline_r, stop_r);

	for (auto r = 0; r < mesh.numRegions; r++) {
		float a = r_distance_a[r] + epsilon / 2;
		float b = r_distance_b[r] + epsilon;
		float c = r_distance_c[r] + epsilon;
		if (a == std::numeric_limits<std::size_t>::max() + epsilon && b == std::numeric_limits<std::size_t>::max() + epsilon) {
			map.r_elevation[r] = 0.1f;
		}
		else {
			map.r_elevation[r] = (1.0f / a - 1.0f / b) / (1.0f / a + 1.0f / b + 1.0f / c);
		}
		map.r_elevation[r] += 0.1f * fbm_noise(map.r_xyz[3 * r], map.r_xyz[3 * r + 1], map.r_xyz[3 * r + 2]);
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

	std::list<priorityQD> _queue;
	int queue_in = 0;
	map.t_downflow_s = std::vector<int>(mesh.numTriangles, -999);
	/* Part 1: ocean triangles get downslope assigned to the lowest neighbor */
	for (int t = 0; t < mesh.numTriangles; t++) {
		if (map.t_elevation[t] < 0) {

			int best_s = -1;
			float best_e = map.t_elevation[t];
			for (int j = 0; j < 3; j++) {
				int s = 3 * t + j;
				float e = map.t_elevation[mesh.s_outer_t(s)];
				if (e < best_e) {
					best_e = e;
					best_s = s;
				}
			}
			map.order_t[queue_in++] = t;
			map.t_downflow_s[t] = best_s;

			_queue.push_back(priorityQD(t, map.t_elevation[t]));
		}
	}
	//sort(_queue->begin(), _queue->end(), compar);
	/* Part 2: land triangles get visited in elevation priority */
	for (int queue_out = 0; queue_out < mesh.numTriangles; queue_out++) {
		int current_t = _queue.front().value;
		_queue.pop_front();
		for (int j = 0; j < 3; j++) {
			int s = 3 * current_t + j;
			int neighbor_t = mesh.s_outer_t(s); // uphill from current_t
			if (map.t_downflow_s[neighbor_t] == -999 && map.t_elevation[neighbor_t] >= 0.0) {
				map.t_downflow_s[neighbor_t] = mesh.s_opposite_s(s);
				map.order_t[queue_in++] = neighbor_t;

				_queue.push_back(priorityQD(neighbor_t, map.t_elevation[neighbor_t]));
			}
		}
	}
}


void Planet::assignFlow() {
	map.s_flow = std::vector<float>(map.s_flow.size(), 0);
	for (int t = 0; t < mesh.numTriangles; t++) {
		if (map.t_elevation[t] >= 0.0f) {
			map.t_flow[t] = 0.5f * map.t_moisture[t] * map.t_moisture[t];
		}
		else {
			map.t_flow[t] = 0;
		}
	}
	for (int i = map.order_t.size() - 1; i >= 0; i--) {
		int tributary_t = map.order_t[i];
		int flow_s = map.t_downflow_s[tributary_t];
		if (flow_s == -1) continue;
		int trunk_t = (mesh.halfedges[flow_s] / 3) | 0;
		if (flow_s >= 0) {
			map.t_flow[trunk_t] += map.t_flow[tributary_t];
			map.s_flow[flow_s] += map.t_flow[tributary_t]; // TODO: isn't s_flow[flow_s] === t_flow[?]
			if (map.t_elevation[trunk_t] > map.t_elevation[tributary_t]) {
				map.t_elevation[trunk_t] = map.t_elevation[tributary_t];
			}
		}
	}
}

/*Distance from any point in seeds_r to all other points, but
* don't go past any point in stop_r */
std::vector<std::size_t> Planet::assignDistanceField(std::vector<std::size_t> seeds_r, std::vector<std::size_t> stop_r) {
	std::vector<std::size_t> r_distance = std::vector<std::size_t>(mesh.numRegions, std::numeric_limits<std::size_t>::max());

	std::vector<std::size_t> queue;
	for (auto r : seeds_r) {
		queue.push_back(r);
		r_distance[r] = 0;
	}

	std::vector<std::size_t> out_r;
	for (int queue_out = 0; queue_out < mesh.numRegions; queue_out++) {
		if (queue.size() == queue_out)
			return r_distance;
		int pos = queue_out + rand() % (queue.size() - queue_out);
		int current_r = queue[pos];
		queue[pos] = queue[queue_out];
		out_r = mesh.r_circulate_r(current_r);
		for (auto neighbor_r : out_r) {
			if (r_distance[neighbor_r] == std::numeric_limits<std::size_t>::max() && !(std::find(stop_r.begin(), stop_r.end(), neighbor_r) != stop_r.end())) {
				r_distance[neighbor_r] = r_distance[current_r] + 1;
				queue.push_back(neighbor_r);
			}
		}
	}
	return r_distance;
	// TODO: possible enhancement: keep track of which seed is closest
	// to this point, so that we can assign variable mountain/ocean
	// elevation to each seed instead of them always being +1/-1
}

Borders Planet::findCollisions() {
	const float deltaTime = 1e-2f; // simulate movement
	const float COLLISION_THRESHOLD = 0.75f;
	std::vector<std::size_t> mountain_r;
	std::vector<std::size_t>	coastline_r;
	std::vector<std::size_t>	ocean_r;
	std::vector<std::size_t>	r_out;
	for (int current_r = 0; current_r < mesh.numRegions; current_r++) {
		float bestCompression = std::numeric_limits<float>::max();
		int best_r = -1;
		r_out = mesh.r_circulate_r(current_r);
		for (auto neighbor_r : r_out) {
			if (map.plates.r_plate[current_r] != map.plates.r_plate[neighbor_r]) {
				vec3 current_pos = vec3(map.r_xyz[3 * current_r], map.r_xyz[3 * current_r + 1], map.r_xyz[3 * current_r + 2]);
				vec3 neighbor_pos = vec3(map.r_xyz[3 * neighbor_r], map.r_xyz[3 * neighbor_r + 1], map.r_xyz[3 * neighbor_r + 2]);
				float distanceBefore = distance(current_pos, neighbor_pos);
				float distanceAfter = distance(current_pos + map.plates.plate_vec[map.plates.r_plate[current_r]] * deltaTime,
					neighbor_pos + map.plates.plate_vec[map.plates.r_plate[neighbor_r]] * deltaTime);

				float compression = distanceBefore - distanceAfter;
				if (compression < bestCompression) {
					best_r = neighbor_r;
					bestCompression = compression;
				}
			}
		}
		if (best_r != -1) {
			bool collided = bestCompression > COLLISION_THRESHOLD * deltaTime;
			if (std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), current_r) != map.plates.plates_is_ocean.end() &&
				std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), best_r) != map.plates.plates_is_ocean.end()) {
				(collided ? coastline_r : ocean_r).push_back(current_r);
			}
			else if (std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), current_r) == map.plates.plates_is_ocean.end() &&
				std::find(map.plates.plates_is_ocean.begin(), map.plates.plates_is_ocean.end(), best_r) == map.plates.plates_is_ocean.end()) {
				if (collided) mountain_r.push_back(current_r);
			}
			else {
				(collided ? mountain_r : coastline_r).push_back(current_r);
			}
		}
	}
	return Borders(mountain_r, coastline_r, ocean_r);
}
