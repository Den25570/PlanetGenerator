#include "PlanetGen.hpp"

std::vector<float> vec3ToXYZ(std::vector<vec3> orig) {
	std::vector<float> res;
	for (auto p : orig)
	{
		res.push_back(p.x);
		res.push_back(p.y);
		res.push_back(p.z);
	}
	return res;
}

/* Calculate the centroid and push it onto an array */
void pushCentroidOfTriangle(std::vector<float> * out, float ax, float ay, float az, float bx, float by, float bz, float cx, float cy, float cz) {
	// TODO: renormalize to radius 1
	out->push_back((ax + bx + cx) / 3);
	out->push_back((ay + by + cy) / 3);
	out->push_back((az + bz + cz) / 3);
}

std::vector<float> generateTriangleCenters(TriangleMesh mesh, std::vector<float> r_xyz) {
	std::vector<float> t_xyz;
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

Map generateMesh(TriangleMesh orig_mesh, int N, int P, int seed, QuadGeometry * quadg) {
	Map map;
	quadg->setMesh(&orig_mesh);

	map.r_xyz = vec3ToXYZ(orig_mesh.points);
	map.t_xyz = generateTriangleCenters(orig_mesh, map.r_xyz);
	map.r_elevation = std::vector<float>(orig_mesh.numRegions);
	map.t_elevation = std::vector<float>(orig_mesh.numTriangles);
	map.r_moisture = std::vector<float>(orig_mesh.numRegions);
	map.t_moisture = std::vector<float>(orig_mesh.numTriangles);
	map.t_downflow_s = std::vector<int>(orig_mesh.numTriangles);
	map.order_t = std::vector<float>(orig_mesh.numTriangles);
	map.t_flow = std::vector<float>(orig_mesh.numTriangles);
	map.s_flow = std::vector<float>(orig_mesh.numSides);

    generateMap(&orig_mesh, &map, quadg,N,P,seed);
	return map;
}

/*Distance from any point in seeds_r to all other points, but
* don't go past any point in stop_r */
std::vector<size_t> assignDistanceField(TriangleMesh * mesh, std::vector<size_t> seeds_r, std::vector<size_t> stop_r) {
	//const int randInt = rand();
	std::vector<size_t> r_distance = std::vector<size_t>(mesh->numRegions, std::numeric_limits<size_t>::max());

	std::vector<size_t> queue;
	for (auto r : seeds_r) {
		queue.push_back(r);
		r_distance[r] = 0;
	}

	std::vector<int> out_r;
	for (int queue_out = 0; queue_out < mesh->numRegions; queue_out++) {
		if (queue.size() == queue_out)
			return r_distance;
		int pos = queue_out + rand() % (queue.size() - queue_out);
		int current_r = queue[pos];
		queue[pos] = queue[queue_out];
		out_r = mesh->r_circulate_r(current_r);
		for (auto neighbor_r : out_r) {
			if (r_distance[neighbor_r] == std::numeric_limits<size_t>::max() && std::find(stop_r.begin(), stop_r.end(), neighbor_r) == stop_r.end()) {
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


/* Calculate the collision measure, which is the amount
 * that any neighbor's plate vector is pushing against
 * the current plate vector. */

Borders* findCollisions(TriangleMesh * mesh, Map * map) {
	const float deltaTime = 1e-2; // simulate movement
	const float COLLISION_THRESHOLD = 0.75;
	std::vector<size_t> mountain_r;
	std::vector<size_t>	coastline_r;
	std::vector<size_t>	ocean_r;
	std::vector<int>	r_out;
	/* For each region, I want to know how much it's being compressed
	   into an adjacent region. The "compression" is the change in
	   distance as the two regions move. I'm looking for the adjacent
	   region from a different plate that pushes most into this one*/
	for (int current_r = 0; current_r < mesh->numRegions; current_r++) {
		float bestCompression = std::numeric_limits<float>::max();
		float best_r = -1;
		r_out = mesh->r_circulate_r(current_r);
		for (auto neighbor_r : r_out) {
			if (map->plates.r_plate[current_r] != map->plates.r_plate[neighbor_r]) {
				/* sometimes I regret storing xyz in a compact array... */
				vec3 current_pos = vec3(map->r_xyz[3 * current_r], map->r_xyz[3 * current_r + 1], map->r_xyz[3 * current_r + 2]);
				vec3 neighbor_pos = vec3(map->r_xyz[3 * neighbor_r], map->r_xyz[3 * neighbor_r + 1], map->r_xyz[3 * neighbor_r + 2]);
				/* simulate movement for deltaTime seconds */
				float distanceBefore = distance(current_pos, neighbor_pos);
				float distanceAfter = distance(current_pos + map->plates.plate_vec[map->plates.r_plate[current_r]] * deltaTime,
			                                   neighbor_pos + map->plates.plate_vec[map->plates.r_plate[neighbor_r]] * deltaTime);
				/* how much closer did these regions get to each other? */
				float compression = distanceBefore - distanceAfter;
				/* keep track of the adjacent region that gets closest */
				if (compression < bestCompression) {
					best_r = neighbor_r;
					bestCompression = compression;
				}
			}
		}
		if (best_r != -1) {
			/* at this point, bestCompression tells us how much closer
			   we are getting to the region that's pushing into us the most */
			int collided = bestCompression > COLLISION_THRESHOLD * deltaTime;
			if (std::find(map->plates.plates_is_ocean.begin(), map->plates.plates_is_ocean.end(), current_r) != map->plates.plates_is_ocean.end() &&
				std::find(map->plates.plates_is_ocean.begin(), map->plates.plates_is_ocean.end(), best_r) != map->plates.plates_is_ocean.end()) {
				(collided ? coastline_r : ocean_r).push_back(current_r);
			}
			else if (std::find(map->plates.plates_is_ocean.begin(), map->plates.plates_is_ocean.end(), current_r) == map->plates.plates_is_ocean.end() &&
				std::find(map->plates.plates_is_ocean.begin(), map->plates.plates_is_ocean.end(), best_r) == map->plates.plates_is_ocean.end()) {
				if (collided) mountain_r.push_back(current_r);
			}
			else {
				(collided ? mountain_r : coastline_r).push_back(current_r);
			}
		}
	}
	return new Borders(mountain_r, coastline_r, ocean_r);
}

float fbm_noise(float nx, float ny, float nz) {
	std::vector<float> amplitudes(5);
	const float persistence = 2.0f / 3.0f;
	for (int i = 0; i < amplitudes.size(); i++)
		amplitudes[i] = (powf(persistence, i));

	float sum = 0, sumOfAmplitudes = 0;
	for (int octave = 0; octave < amplitudes.size(); octave++) {
		auto frequency = 1 << octave;
		sum += amplitudes[octave] * SimplexNoise::noise(nx * frequency, ny * frequency, nz * frequency);
		sumOfAmplitudes += amplitudes[octave];
	}
	return sum / sumOfAmplitudes;
}

void assignRegionElevation(TriangleMesh * mesh, Map * map) {
	const float epsilon = 1e-3;

	auto borders = *findCollisions(mesh, map);

	for (int r = 0; r < mesh->numRegions; r++) {
		if (map->plates.r_plate[r] == r) {
			(std::find(map->plates.plates_is_ocean.begin(), map->plates.plates_is_ocean.end(), r) != map->plates.plates_is_ocean.end() ?
				borders.ocean_r : borders.coastline_r).push_back(r);
		}
	}

	std::vector<size_t> stop_r;
	for (auto r : borders.mountain_r) { stop_r.push_back(r); }
	for (auto r : borders.coastline_r) { stop_r.push_back(r); }
	for (auto r : borders.ocean_r) { stop_r.push_back(r); }

	std::cout << "seeds mountain/coastline/ocean:" << borders.mountain_r.size() << " " << borders.coastline_r.size() << " " << borders.ocean_r.size() << " " << "plate_is_ocean: " << map->plates.plates_is_ocean.size() << std::endl;
	auto r_distance_a = assignDistanceField(mesh, borders.mountain_r, borders.ocean_r);
	auto r_distance_b = assignDistanceField(mesh, borders.ocean_r, borders.coastline_r);
	auto r_distance_c = assignDistanceField(mesh, borders.coastline_r, stop_r);

	for (auto r = 0; r < mesh->numRegions; r++) {
		float a = r_distance_a[r] + epsilon;
		float b = r_distance_b[r] + epsilon;
		float c = r_distance_c[r] + epsilon;
		if (a == std::numeric_limits<float>::max() && b == std::numeric_limits<float>::max()) {
			map->r_elevation[r] = 0.1;
		}
		else {
			map->r_elevation[r] = (1 / a - 1 / b) / (1 / a + 1 / b + 1 / c);
		}
		map->r_elevation[r] += 0.1 * fbm_noise(map->r_xyz[3 * r], map->r_xyz[3 * r + 1], map->r_xyz[3 * r + 2]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void assignDownflow(TriangleMesh* mesh, Map* map, std::queue<int>* _queue) {
	/* Use a priority queue, starting with the ocean triangles and
	 * moving upwards using elevation as the priority, to visit all
	 * the land triangles */
	int queue_in = 0;
	map->t_downflow_s = std::vector<int>(mesh->numTriangles, -999);
	/* Part 1: ocean triangles get downslope assigned to the lowest neighbor */
	for (int t = 0; t < mesh->numTriangles; t++) {
		if (map->t_elevation[t] < 0) {
			int best_s = -1;
			int best_e = map->t_elevation[t];
			for (int j = 0; j < 3; j++) {
				int s = 3 * t + j;
				int	e = map->t_elevation[mesh->s_outer_t(s)];
				if (e < best_e) {
					best_e = e;
					best_s = s;
				}
			}
			map->order_t[queue_in++] = t;
			map->t_downflow_s[t] = best_s;
			_queue->push(t);
			_queue->push(map->t_elevation[t]);
		}
	}
	/* Part 2: land triangles get visited in elevation priority */
	for (int queue_out = 0; queue_out < mesh->numTriangles; queue_out++) {
		int current_t = _queue->front();
		_queue->pop();
		for (int j = 0; j < 3; j++) {
			int s = 3 * current_t + j;
			int neighbor_t = mesh->s_outer_t(s); // uphill from current_t
			if (map->t_downflow_s[neighbor_t] == -999 && map->t_elevation[neighbor_t] >= 0.0) {
				map->t_downflow_s[neighbor_t] = mesh->s_opposite_s(s);
				map->order_t[queue_in++] = neighbor_t;
				_queue->push(neighbor_t);
				_queue->push(map->t_elevation[neighbor_t]);
			}
		}
	}
}


void assignFlow(TriangleMesh* mesh, Map* map) {
	map->s_flow = std::vector<float>(0, 0);
	for (int t = 0; t < mesh->numTriangles; t++) {
		if (map->t_elevation[t] >= 0.0) {
			map->t_flow[t] = 0.5 * map->t_moisture[t] * map->t_moisture[t];
		}
		else {
			map->t_flow[t] = 0;
		}
	}
	for (int i = map->order_t.size() - 1; i >= 0; i--) {
		int tributary_t = map->order_t[i];
		int flow_s = map->t_downflow_s[tributary_t];
		int trunk_t = (mesh->halfedges[flow_s] / 3) | 0;
		if (flow_s >= 0) {
			map->t_flow[trunk_t] += map->t_flow[tributary_t];
			map->s_flow[flow_s] += map->t_flow[tributary_t]; // TODO: isn't s_flow[flow_s] === t_flow[?]
			if (map->t_elevation[trunk_t] > map->t_elevation[tributary_t]) {
				map->t_elevation[trunk_t] = map->t_elevation[tributary_t];
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

void assignTriangleValues(TriangleMesh* mesh, Map* map) {
	for (int t = 0; t < mesh->numTriangles; t++) {
		int s0 = 3 * t;
		int r1 = mesh->s_begin_r(s0);
		int	r2 = mesh->s_begin_r(s0 + 1);
		int	r3 = mesh->s_begin_r(s0 + 2);
		map->t_elevation[t] = 1 / 3 * (map->r_elevation[r1] + map->r_elevation[r2] + map->r_elevation[r3]);
		map->t_moisture[t] = 1 / 3 * (map->r_moisture[r1] + map->r_moisture[r2] + map->r_moisture[r3]);
	}
}

void generateMap(TriangleMesh * mesh, Map * map, QuadGeometry * quadGeometry, int N, int P, int seed) {
	Plates * plates = generatePlates(mesh, map->r_xyz, N,P,seed);
	map->plates = *plates;

	for (auto r : map->plates.region_set) {
		if (rand() % (10) < 5) {
			map->plates.plates_is_ocean.push_back(r);
			// TODO: either make tiny plates non-ocean, or make sure tiny plates don't create seeds for rivers
		}
	}
	assignRegionElevation(mesh, map);
	// TODO: assign region moisture in a better way!
	for (int r = 0; r < mesh->numRegions; r++) {
		map->r_moisture[r] = (map->plates.r_plate[r] % 10) / 10.0;
	}
	std::queue<int> _queue;

	assignTriangleValues(mesh, map);
	//assignDownflow(&mesh, &map, &_queue);
	//assignFlow(&mesh, &map);

	quadGeometry->setMap(mesh, map);
}



std::vector<size_t> pickRandomRegions(int numRegions, int platesNum, int seed) {
	srand(seed);
	std::vector<size_t> region_set;
	while (region_set.size() < platesNum && region_set.size() < numRegions) {
		region_set.push_back(rand() % numRegions);
	}
	return region_set;
}

Plates * generatePlates(TriangleMesh * mesh, std::vector<float> points, int N, int P, int seed) {
	std::vector<int> r_plate = std::vector<int>(mesh->numRegions, -1);

	auto region_set = pickRandomRegions(mesh->numRegions, std::min(N, P), seed);
	auto queue = region_set;

	for (auto r : queue) { r_plate[r] = r; }
	std::vector<size_t> out_r;

	for (int queue_out = 0; queue_out < mesh->numRegions; queue_out++) {
		int pos = queue_out + rand() % (queue.size() - queue_out);
		int current_r = queue[pos];
		queue[pos] = queue[queue_out];
		auto out_r = mesh->r_circulate_r(current_r);
		for (auto neighbor_r : out_r) {
			if (r_plate[neighbor_r] == -1) {
				r_plate[neighbor_r] = r_plate[current_r];
				queue.push_back(neighbor_r);
			}
		}
	}

	// Assign a random movement vector for each plate
	std::vector<vec3> plate_vec = std::vector<vec3>(N);
	for (auto center_r : region_set) {
		auto neighbor_r = mesh->r_circulate_r(center_r)[0];
		vec3 p0 = vec3(points[3 * center_r], points[3 * center_r + 1], points[3 * center_r + 2]);
		vec3 p1 = vec3(points[3 * neighbor_r], points[3 * neighbor_r + 1], points[3 * neighbor_r + 2]);
		plate_vec[center_r] = normalize(p1 - p0);
	}

	for (int i = 0; i < r_plate.size(); i++)
		if (r_plate[i] == -1)
			r_plate[i] = r_plate[rand() % region_set.size()];

	return new Plates(r_plate, plate_vec, region_set);
}


TriangleMesh generateDelanuaySphere(std::vector<vec3>* verticles) {
	Delaunator delaunay = Delaunator(vec2ToDouble(stereographicProjection((verticles))));

	std::vector<vec3> points;
	for (std::size_t i = 0; i < verticles->size(); i++)
		points.push_back((*verticles)[i]);

    points.push_back(vec3(0.0f, 0.0f, 1.0f));
	addSouthPoleToMesh(points.size() - 1, &delaunay, &points);

	auto dummy_r_vertex = std::vector<std::vector<size_t>>(verticles->size() + 1, std::vector<size_t>(2, 0));

	TriangleMesh mesh = TriangleMesh(0,
		delaunay.triangles.size(),
		dummy_r_vertex,
		delaunay.triangles,
		delaunay.halfedges,
		points
	);

	return mesh;
}

std::vector<double> vec2ToDouble(std::vector<vec2> * points) {
	std::vector<double> res;
	for (auto p : *points)
	{
		res.push_back((double)p.x);
		res.push_back((double)p.y);
	}
	return res;
}

void addSouthPoleToMesh(uint southPoleId, Delaunator * delanuay, std::vector<vec3> * p) {

	/*for (int i = 0; i < p->size() ; i++)
		if (compareF((*p)[i].x, 0, 1e-5) && compareF((*p)[i].y, 0, 1e-5) && compareF((*p)[i].z, 1, 1e-5))
		{
			(*p)[i].z = -(*p)[i].z;
			break;
		}*/

	int numSides = delanuay->triangles.size();

	int numUnpairedSides = 0;
	int firstUnpairedSide = -1;

	std::vector<size_t> pointIdToSideId = std::vector<size_t>(numSides);

	for (std::size_t s = 0; s < numSides; s++) {
		if (delanuay->halfedges[s] == -1) {
			numUnpairedSides++;
			pointIdToSideId[delanuay->triangles[s]] = s;
			firstUnpairedSide = s;
		}
	}

	std::vector<size_t> newTriangles = std::vector<size_t>(numSides + 3 * numUnpairedSides);
	std::vector<size_t> newHalfedges = std::vector<size_t>(numSides + 3 * numUnpairedSides);
	std::copy(delanuay->triangles.begin(), delanuay->triangles.end(), newTriangles.begin());
	std::copy(delanuay->halfedges.begin(), delanuay->halfedges.end(), newHalfedges.begin());


	for (std::size_t i = 0, s = firstUnpairedSide;
		i < numUnpairedSides;
		i++, s = pointIdToSideId[newTriangles[(s % 3 == 2) ? s - 2 : s + 1]]) {

		// Construct a pair for the unpaired side s
		std::size_t newSide = numSides + 3 * i;
		newHalfedges[s] = newSide;
		newHalfedges[newSide] = s;
		newTriangles[newSide] = newTriangles[(s % 3 == 2) ? s - 2 : s + 1];

		// Construct a triangle connecting the new side to the south pole
		newTriangles[newSide + 1] = newTriangles[s];
		newTriangles[newSide + 2] = southPoleId;
		std::size_t k = numSides + (3 * i + 4) % (3 * numUnpairedSides);
		newHalfedges[newSide + 2] = k;
		newHalfedges[k] = newSide + 2;
	}
	delanuay->triangles = newTriangles;
	delanuay->halfedges = newHalfedges;
}

void draw(int N) {
	float u_pointsize = 0.1 + 100 / sqrtf(N);
	//mat4 u_projection;
	//scale(u_projection, u_projection, [1, 1, 0.5, 1]); // avoid clipping
	//rotate(u_projection, u_projection, -rotation, [0.1, 1, 0]);
	//rotate(u_projection, u_projection, -Math.PI / 2 + 0.2, [1, 0, 0]);
/*
	function r_color_fn(r) {
		let m = map.r_moisture[r];
		let e = map.r_elevation[r];
		return[e, m];
	}
*/
//if (drawMode == = 'centroid') {
//	let triangleGeometry = generateVoronoiGeometry(mesh, map, r_color_fn);
//	renderTriangles({
//		u_projection,
//		a_xyz: triangleGeometry.xyz,
//		a_tm : triangleGeometry.tm,
//		count : triangleGeometry.xyz.length / 3,
//		});
//}

/*
	else if (drawMode == = 'quads') {
		renderIndexedTriangles({
			u_projection,
			a_xyz: quadGeometry.xyz,
			a_tm : quadGeometry.tm,
			elements : quadGeometry.I,
			});
	}
	*/

	//drawRivers(u_projection, mesh, map);

	//if (draw_plateVectors) {
	//	drawPlateVectors(u_projection, mesh, map);
	//}
	//if (draw_plateBoundaries) {
	//	drawPlateBoundaries(u_projection, mesh, map);
	//}

	// renderPoints({
	//     u_projection,
	//     u_pointsize,
	//     a_xyz: map.r_xyz,
	//     count: mesh.numRegions,
	// });
}

