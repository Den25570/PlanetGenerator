#include "ExtendedMath.hpp"
#include "mesh.h"
#include <queue>
#include <algorithm>
#include <limits>
#include "StructToFlat.hpp"

Isocahedron::Isocahedron(int subdivisions_num, float radius, bool onlyPoints)
{
	float phi = (1.0 + sqrt(5.0)) / 2.0;
	float du = 1.0 / sqrt(phi * phi + 1.0);
	float dv = phi * du;

	verticles =
	{
		 vec3(0, +dv, +du),
		 vec3(0, +dv, -du),
		 vec3(0, -dv, +du),
		 vec3(0, -dv, -du),

		 vec3(+du, 0, +dv),
		 vec3(-du, 0, +dv),
		 vec3(+du, 0, -dv),
		 vec3(-du, 0, -dv),

		 vec3(+dv, +du, 0),
		 vec3(+dv, -du, 0),
		 vec3(-dv, +du, 0),
		 vec3(-dv, -du, 0),
	};

	triangles =
	{
		{0, 1, 8  },
		{0,  4,  5},
		{0,  5, 10},
		{0,  8,  4},
		{0, 10,  1},
		{1,  6,  8},
		{1,  7,  6},
		{1, 10,  7},
		{2,  3, 11},
		{2,  4,  9},
		{2,  5,  4},
		{2,  9,  3},
		{5, 2,  11},
		{6,  7,  3},
		{7,  11, 3},
		{9,  6,  3},
		{4,  8,  9},
		{11, 10, 5},
		{6,  9,  8},
		{7, 10, 11},
	};
	generateSubdivisions_recursive(subdivisions_num);
	normalize(radius);
}

//Рекурсивное подразделение треугольников исокаэдра
void Isocahedron::generateSubdivisions_recursive(int n)
{
	for (int i = 0; i < n; i++)
	{
		int j = 0;
		int end = triangles.size();

		for (auto it = triangles.begin(); j < end; it++, j++)
		{
			vec3 v0 = verticles[(*it)[0]];
			vec3 v1 =  verticles[(*it)[1]];
			vec3 v2 =  verticles[(*it)[2]];

			//Создание новых вершин, если уже существует, то использовать имеющуюся
			vec3 v3 = (v0 + v1) * 0.5f;
			vec3 v4 = (v1 + v2) * 0.5f;
			vec3 v5 = (v0 + v2) * 0.5f;;

			int vu3 = checkAndAddVertex(&verticles, v3);
			int vu4 = checkAndAddVertex(&verticles, v4);
			int vu5 = checkAndAddVertex(&verticles, v5);

			//Добавление новых треугольников
			triangles.push_back({ static_cast<unsigned short>(vu4), (*it)[2], static_cast<unsigned short>(vu5) });
		    triangles.push_back({ static_cast<unsigned short>(vu3), (*it)[1], static_cast<unsigned short>(vu4) });
		    triangles.push_back({ static_cast<unsigned short>(vu3), static_cast<unsigned short>(vu4), static_cast<unsigned short>(vu5) });
			*it = std::vector<unsigned short>{ (*it)[0], static_cast<unsigned short>(vu3), static_cast<unsigned short>(vu5) };
		}
	}
}

// Нерекурсивное подразделение треугольников исокаэдра n > 1
void Isocahedron::generateSubdivisions_nonRecursive(int n)
{
	int j = 0;
	int size = triangles.size();

	for (auto it = triangles.begin(); j < size; it++, j++)
	{
		std::queue<unsigned short> queue;
		std::vector<vec3> bottom_vertexes;
		std::vector<vec3> bottom_vertexes_rev;

		float Len = length(verticles[(*it)[0]] - verticles[(*it)[1]]) / n;

		//Вектор дополнительных точек треугольника
		bottom_vertexes.push_back(verticles[(*it)[1]]);
		for (int i = 1; i < n; i++)
			bottom_vertexes.push_back(calcVector(verticles[(*it)[1]], verticles[(*it)[2]], Len*i));
		bottom_vertexes.push_back(verticles[(*it)[2]]);

		bottom_vertexes_rev = bottom_vertexes;
		std::reverse(bottom_vertexes_rev.begin(), bottom_vertexes_rev.end());

		std::vector<unsigned short> added_vertexes_indexes_prev{ (*it)[0] };

		int j = 0;
		queue.push((*it)[0]);
		for (int i = 0; i < n; i++)
		{
			std::vector<unsigned short> added_vertexes_indexes;
			for (int j = 0; j <= i; j++)
			{
				//Создание новой пары вершин
				unsigned short index = queue.front();
				queue.pop();

				vec3 v0 = calcVector(verticles[index], bottom_vertexes[j], Len);
				vec3 v1 = calcVector(verticles[index], bottom_vertexes_rev[i - j], Len);

				int vu0 = checkAndAddVertex(&verticles, v0);
				int vu1 = checkAndAddVertex(&verticles, v1);

				//Добавление новых вершин в очередь
				if (std::find(added_vertexes_indexes.begin(), added_vertexes_indexes.end(), vu0) == added_vertexes_indexes.end()) { added_vertexes_indexes.push_back(static_cast<unsigned short>(vu0)); queue.push(vu0); }
				if (std::find(added_vertexes_indexes.begin(), added_vertexes_indexes.end(), vu1) == added_vertexes_indexes.end()) { added_vertexes_indexes.push_back(static_cast<unsigned short>(vu1)); queue.push(vu1);}					
			}

			//Создание новых треугольников
			int vertex_arr_size = added_vertexes_indexes_prev.size();
			for (int j = 0; j < vertex_arr_size; j++)
			{
				triangles.push_back({ added_vertexes_indexes_prev[j],added_vertexes_indexes[j],added_vertexes_indexes[j+1]});
				if (j < vertex_arr_size - 1)
					triangles.push_back({ added_vertexes_indexes_prev[j+1],added_vertexes_indexes_prev[j],added_vertexes_indexes[j + 1] });
				//Замена предыдущих добавленных вершин на текущие
				added_vertexes_indexes_prev[j] = added_vertexes_indexes[j];
			}
			//Вектор предыдущих вершин должен быть на одну больше 
			added_vertexes_indexes_prev.push_back(added_vertexes_indexes[vertex_arr_size]);
		}
		//Заменяем данные текущего треугольника последним сгенерированным
		*it = triangles.back();
		triangles.pop_back();
	}
}

void Isocahedron::normalize(float sphereRadius)
{
	for (auto it = verticles.begin(); it != verticles.end(); it++)
	{
		float length = sqrtf(it->x * it->x + it->y * it->y  + it->z * it->z);
		(*it) *= sphereRadius / length;
	}
}

int Isocahedron::checkAndAddVertex(std::vector<vec3> *verticles, vec3 vertex)
{
	int vu = vertexExist(verticles, vertex);
	if (vu == -1)
	{
		vu = verticles->size();
		verticles->push_back(vertex);
	}
	return vu;
}

int Isocahedron::vertexExist(std::vector<vec3> *verticles, vec3 vertex)
{
	for (int i = 0; i < verticles->size(); i++)
		if (compareF((*verticles)[i].x, vertex.x) && compareF((*verticles)[i].y, vertex.y) && compareF((*verticles)[i].z, vertex.z))
			return i;
	return -1;
}

int Isocahedron::triangleExist(std::list<std::vector<unsigned short>> *triangles, std::vector<unsigned short> triangle)
{
	int i = 0;
	for (auto it = triangles->cbegin(); it != triangles->cend(); it++, i++)
	{
		if ((*it)[0] == triangle[0] && (*it)[1] == triangle[1] && (*it)[2] == triangle[2])
			return i;
	}
	return -1;
}

void TriangleMesh::update() {
	auto _triangles = this->triangles;
	auto _halfedges = this->halfedges;
	auto _r_vertex = this->verticles;
	auto _t_vertex = this->t_verticles;

	numSides = _triangles.size();
	numRegions = _r_vertex.size();
	numSolidRegions = numRegions - 1; // TODO: only if there are ghosts
	numTriangles = numSides / 3;
	numSolidTriangles = numSolidSides / 3;

	if (t_verticles.size() < numTriangles) 
	{
		// Extend this array to be big enough
		const int numOldTriangles = _t_vertex.size();
		const int numNewTriangles = numTriangles - numOldTriangles;

		_t_vertex.resize(numOldTriangles + numNewTriangles);
		for (int t = numOldTriangles; t < numTriangles; t++)
			_t_vertex[t] = std::vector<std::size_t>(2, 0);
		t_verticles = _t_vertex;
	}

	// Construct an index for finding sides connected to a region
	r_in_s = std::vector<int>(numRegions);
	for (int s = 0; s < _triangles.size(); s++) {
		int endpoint = _triangles[next_site(s)];
		if (r_in_s[endpoint] == 0 || _halfedges[s] == -1) {
			r_in_s[endpoint] = s;
		}
	}

	// Construct triangle coordinates
	for (int s = 0; s < _triangles.size(); s += 3) {
		int t = s / 3;
		std::vector<std::size_t> a = _r_vertex[_triangles[s]];
		std::vector<std::size_t> b = _r_vertex[_triangles[s + 1]];
		std::vector<std::size_t> c = _r_vertex[_triangles[s + 2]];
		if (ghost_sites(s)) {
			// ghost triangle center is just outside the unpaired side
			std::size_t dx = b[0] - a[0];
			std::size_t dy = b[1] - a[1];
			float scale = 10 / sqrtf(dx*dx + dy * dy); // go 10units away from side
			_t_vertex[t][0] = 0.5 * (a[0] + b[0]) + dy * scale;
			_t_vertex[t][1] = 0.5 * (a[1] + b[1]) - dx * scale;
		}
		else {
			// solid triangle center is at the centroid
			_t_vertex[t][0] = (a[0] + b[0] + c[0]) / 3;
			_t_vertex[t][1] = (a[1] + b[1] + c[1]) / 3;
		}
	}
}

void TriangleMesh::Update_d(std::vector<std::vector<std::size_t>> _points, std::vector<std::size_t> _triangles, std::vector<std::size_t> _halfedges) {
	verticles = _points;
	triangles = _triangles;
	halfedges = _halfedges;

	update();
}

int TriangleMesh::site_to_triangle(int site) { return (site / 3) | 0; }
int TriangleMesh::prev_site(int site) { return (site % 3 == 0) ? site + 2 : site - 1; }
int TriangleMesh::next_site(int site) { return (site % 3 == 2) ? site - 2 : site + 1; }

int TriangleMesh::ghost_r() { return numRegions - 1; }
int TriangleMesh::ghost_sites(int site) { return site >= numSolidSides; }
int TriangleMesh::ghost_regions(int r) { return r == numRegions - 1; }
int TriangleMesh::ghost_triangles(int t) { return ghost_sites(3 * t); }
int TriangleMesh::s_boundary(int site) { return ghost_sites(site) && (site % 3 == 0); }
int TriangleMesh::r_boundary(int r) { return r < numBoundaryRegions; }

int TriangleMesh::region_x(int r) { return verticles[r][0]; }
int TriangleMesh::region_y(int r) { return verticles[r][1]; }
int TriangleMesh::triangle_center_x(int r) { return t_verticles[r][0]; }
int TriangleMesh::triangle_center_y(int r) { return t_verticles[r][1]; }
std::vector<int> TriangleMesh::region_position(int r) { auto out = std::vector<int>(2); out[0] = region_x(r); out[1] = region_y(r); return out; }
std::vector<int> TriangleMesh::triangle_center_position(int t) { auto out = std::vector<int>(2); out[0] = triangle_center_x(t); out[1] = triangle_center_y(t); return out; }

int TriangleMesh::start_site_of_region(int site) { return triangles[site]; }
int TriangleMesh::s_end_r(int site) { return triangles[next_site(site)]; }

int TriangleMesh::inner_triangle(int site) { return site_to_triangle(site); }
int TriangleMesh::outer_triangle(int site) { return site_to_triangle(halfedges[site]); }

int TriangleMesh::s_opposite_s(int site) { return halfedges[site]; }

std::vector<std::size_t> TriangleMesh::t_circulate_s(std::size_t t) { auto result = std::vector<std::size_t>(3); for (std::size_t i = 0; i < 3; i++) { result[i] = 3 * t + i; } return result; }
std::vector<std::size_t> TriangleMesh::t_circulate_r(std::size_t t) { auto result = std::vector<std::size_t>(3); for (std::size_t i = 0; i < 3; i++) { result[i] = triangles[3 * t + i]; } return result; }
std::vector<std::size_t> TriangleMesh::t_circulate_t(std::size_t t) { auto result = std::vector<std::size_t>(3); for (std::size_t i = 0; i < 3; i++) { result[i] = outer_triangle(3 * t + i); } return result; }

std::vector<std::size_t> TriangleMesh::region_sites(std::size_t r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<std::size_t> result;
	do {
		result.push_back(halfedges[incoming]);
		auto outgoing = next_site(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return result;
}

std::vector<std::size_t> TriangleMesh::neighbour_regions(std::size_t r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<std::size_t> result;
	do {
		result.push_back(start_site_of_region(incoming));
		auto outgoing = next_site(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return result;
}

std::vector<std::size_t> TriangleMesh::neighbour_triangle_centers(std::size_t r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<std::size_t> result;
	do {
		result.push_back(site_to_triangle(incoming));
		auto outgoing = next_site(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return result;
}

float randFloat()
{
	return ((float)rand() / RAND_MAX);
}

std::vector<vec3> generateFibonacciSphere(int N, float jitter) {
	std::vector<float> a_latlong;
	std::vector<float> rand_latitude_shift = std::vector<float>(N, std::numeric_limits<int>::max());
	std::vector<float> rand_longitude_shift = std::vector<float>(N, std::numeric_limits<int>::max());

	const float s = 3.6f / sqrtf(N);
	const float dz = 2.0f / N;
	for (float k = 0, lng = 0, z = 1 - dz / 2; k != N; k++, z -= dz) {
		float r = sqrtf(1.0f - z * z);
		float latitude_deg = asinf(z) * 180.0f / M_PI;
		float longitude_deg = lng * 180.0f / M_PI;
		if (rand_latitude_shift[k] == std::numeric_limits<int>::max()) rand_latitude_shift[k] = randFloat() - randFloat();
		if (rand_longitude_shift[k] == std::numeric_limits<int>::max()) rand_longitude_shift[k] = randFloat() - randFloat();
		latitude_deg += jitter * rand_latitude_shift[k] * (latitude_deg - asin(max(-1.0, z - dz * 2 * M_PI * r / s)) * 180 / M_PI);
		longitude_deg += jitter * rand_longitude_shift[k] * (s / r * 180.0 / M_PI);
		a_latlong.push_back(latitude_deg);
		a_latlong.push_back(((int)trunc(longitude_deg) % 360) + longitude_deg - trunc(longitude_deg));
		lng += s / r;
	}

	std::vector<vec3> out;
	for (int r = 0; r < a_latlong.size() / 2; r++) {
		float latRad = a_latlong[r * 2 + 0] / 180.0f * M_PI;
		float lonRad = a_latlong[r * 2 + 1] / 180.0f * M_PI;
		out.push_back(vec3(cosf(latRad) * cosf(lonRad),
			cosf(latRad) * sinf(lonRad),
			sinf(latRad)));
	}
	return out;
}

TriangleMesh generateDelanuaySphere(std::vector<vec3>* verticles) {
	Delaunator delaunay = Delaunator(vec2ToDouble(stereographicProjection((verticles))));

	std::vector<vec3> points;
	for (std::size_t i = 0; i < verticles->size(); i++)
		points.push_back((*verticles)[i]);

	points.push_back(vec3(0.0f, 0.0f, 1.0f));
    addSouthPoleToMesh(points.size() - 1, &delaunay, &points);

	auto dummy_r_vertex = std::vector<std::vector<std::size_t>>(verticles->size() + 1, std::vector<std::size_t>(2, 0));

	TriangleMesh mesh = TriangleMesh(0,
		delaunay.triangles.size(),
		dummy_r_vertex,
		delaunay.triangles,
		delaunay.halfedges,
		points
	);

	return mesh;
}

void addSouthPoleToMesh(std::size_t southPoleId, Delaunator * delanuay, std::vector<vec3> * p) {
	int numSides = delanuay->triangles.size();

	int numUnpairedSides = 0;
	int firstUnpairedSide = -1;

	std::vector<std::size_t> pointIdToSideId = std::vector<std::size_t>(numSides);

	for (std::size_t s = 0; s < numSides; s++) {
		if (delanuay->halfedges[s] == -1) {
			numUnpairedSides++;
			pointIdToSideId[delanuay->triangles[s]] = s;
			firstUnpairedSide = s;
		}
	}

	std::vector<std::size_t> newTriangles = std::vector<std::size_t>(numSides + 3 * numUnpairedSides);
	std::vector<std::size_t> newHalfedges = std::vector<std::size_t>(numSides + 3 * numUnpairedSides);
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