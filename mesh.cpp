#include "ExtendedMath.hpp"
#include "mesh.h"
#include <queue>

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
		it->x *= sphereRadius / length;
		it->y *= sphereRadius / length;
		it->z *= sphereRadius / length;
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
	auto _halfedges = halfedges;
	auto _r_vertex = verticles;
	auto _t_vertex = t_verticles;

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
		int endpoint = _triangles[s_next_s(s)];
		if (r_in_s[endpoint] == 0 || _halfedges[s] == -1) {
			r_in_s[endpoint] = s;
		}
	}

	// Construct triangle coordinates
	for (int s = 0; s < _triangles.size(); s += 3) {
		int t = s / 3;
		std::vector<size_t> a = _r_vertex[_triangles[s]];
		std::vector<size_t> b = _r_vertex[_triangles[s + 1]];
		std::vector<size_t> c = _r_vertex[_triangles[s + 2]];
		if (s_ghost(s)) {
			// ghost triangle center is just outside the unpaired side
			size_t dx = b[0] - a[0];
			size_t dy = b[1] - a[1];
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

void TriangleMesh::Update_d(std::vector<std::vector<size_t>> _points, std::vector<std::size_t> _triangles, std::vector<std::size_t> _halfedges) {
	verticles = _points;
	triangles = _triangles;
	halfedges = _halfedges;

	update();
}

int TriangleMesh::s_to_t(int s) { return (s / 3) | 0; }
int TriangleMesh::s_prev_s(int s) { return (s % 3 == 0) ? s + 2 : s - 1; }
int TriangleMesh::s_next_s(int s) { return (s % 3 == 2) ? s - 2 : s + 1; }

int TriangleMesh::ghost_r() { return numRegions - 1; }
int TriangleMesh::s_ghost(int s) { return s >= numSolidSides; }
int TriangleMesh::r_ghost(int r) { return r == numRegions - 1; }
int TriangleMesh::t_ghost(int t) { return s_ghost(3 * t); }
int TriangleMesh::s_boundary(int s) { return s_ghost(s) && (s % 3 == 0); }
int TriangleMesh::r_boundary(int r) { return r < numBoundaryRegions; }

int TriangleMesh::r_x(int r) { return verticles[r][0]; }
int TriangleMesh::r_y(int r) { return verticles[r][1]; }
int TriangleMesh::t_x(int r) { return t_verticles[r][0]; }
int TriangleMesh::t_y(int r) { return t_verticles[r][1]; }
std::vector<int> TriangleMesh::r_pos(int r) { auto out = std::vector<int>(2); out[0] = r_x(r); out[1] = r_y(r); return out; }
std::vector<int> TriangleMesh::t_pos(int t) { auto out = std::vector<int>(2); out[0] = t_x(t); out[1] = t_y(t); return out; }

int TriangleMesh::s_begin_r(int s) { return triangles[s]; }
int TriangleMesh::s_end_r(int s) { return triangles[s_next_s(s)]; }

int TriangleMesh::s_inner_t(int s) { return s_to_t(s); }
int TriangleMesh::s_outer_t(int s) { return s_to_t(halfedges[s]); }

int TriangleMesh::s_opposite_s(int s) { return halfedges[s]; }

std::vector<int> TriangleMesh::t_circulate_s(int t) { auto out_s = std::vector<int>(3); for (int i = 0; i < 3; i++) { out_s[i] = 3 * t + i; } return out_s; }
std::vector<int> TriangleMesh::t_circulate_r(int t) { auto out_r = std::vector<int>(3); for (int i = 0; i < 3; i++) { out_r[i] = triangles[3 * t + i]; } return out_r; }
std::vector<int> TriangleMesh::t_circulate_t(int t) { auto out_t = std::vector<int>(3); for (int i = 0; i < 3; i++) { out_t[i] = s_outer_t(3 * t + i); } return out_t; }

std::vector<int> TriangleMesh::r_circulate_s(int r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<int> out_s;
	do {
		out_s.push_back(halfedges[incoming]);
		auto outgoing = s_next_s(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_s;
}

std::vector<int> TriangleMesh::r_circulate_r(int r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<int> out_r;
	do {
		out_r.push_back(s_begin_r(incoming));
		auto outgoing = s_next_s(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_r;
}

std::vector<int> TriangleMesh::r_circulate_t(int r) {
	const int s0 = r_in_s[r];
	auto incoming = s0;
	std::vector<int> out_t;
	do {
		out_t.push_back(s_to_t(incoming));
		auto outgoing = s_next_s(incoming);
		incoming = halfedges[outgoing];
	} while (incoming != -1 && incoming != s0);
	return out_t;
}