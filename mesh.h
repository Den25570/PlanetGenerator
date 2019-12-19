#pragma once

#include  <glm/glm.hpp>

#include <vector>
#include <list>
#include "delaunator.hpp"

using namespace delaunator;

struct Isocahedron
{
	std::vector<vec3> verticles;
	std::list<std::vector<unsigned short>> triangles;

	Isocahedron() {};
    Isocahedron(int subdivisions_num, float radius, bool onlyPoints = false);

private:
    void generateSubdivisions_recursive(int n);
	void generateSubdivisions_nonRecursive(int n);
	void normalize(float sphereRadius);
	
	int checkAndAddVertex(std::vector<vec3> *verticles, vec3 vertex);
	int vertexExist(std::vector<vec3> *verticles, vec3 vertex);
	int triangleExist(std::list<std::vector<unsigned short>> *triangles, std::vector<unsigned short> triangle);
};

//TODO: Пересмотреть структуру, переименовать переменные
struct TriangleMesh {
	int numBoundaryRegions;
	int numSolidSides;

	std::vector<std::vector<std::size_t>> verticles; //r
	std::vector<std::size_t> triangles;              //t
	std::vector<std::size_t> halfedges;              //s
	std::vector<std::vector<std::size_t>> t_verticles;

	int numSides;
	int numRegions;
	int numSolidRegions;
	int numTriangles;
	int numSolidTriangles;
	std::vector<int> r_in_s;

	std::vector<vec3> points;

	TriangleMesh() {};
	TriangleMesh(int _numBoundaryRegions,
		int _numSolidSides,
		std::vector<std::vector<std::size_t>> _verticles,
		std::vector<std::size_t> _triangles,
		std::vector<std::size_t> _halfedges,
		std::vector<vec3> _points)
	{
	    numBoundaryRegions = _numBoundaryRegions;
	    numSolidSides = _numSolidSides;
		verticles = _verticles;
		triangles = _triangles;
		halfedges = _halfedges;
		points = _points;		

		update();
	}

	~TriangleMesh() {
		verticles.clear();
		triangles.clear();
		halfedges.clear();
		points.clear();
	}

	void Update_d(std::vector<std::vector<std::size_t>> _points, std::vector<std::size_t> _triangles, std::vector<std::size_t> _halfedges);

	void update();

	static int site_to_triangle(int s);
	static int prev_site(int s);
	static int next_site(int s);

	int ghost_r();
	int ghost_sites(int s);
	int ghost_regions(int r);
	int ghost_triangles(int t);
	int s_boundary(int s);
	int r_boundary(int r);

	int region_x(int r);
	int region_y(int r);
	int triangle_center_x(int r);
	int triangle_center_y(int r);
	std::vector<int> region_position(int r);
	std::vector<int> triangle_center_position(int t);

	int start_site_of_region(int s);
	int s_end_r(int s);

	int inner_triangle(int s);
	int outer_triangle(int s);

	int s_opposite_s(int s);

	std::vector<std::size_t> t_circulate_s(std::size_t t);
	std::vector<std::size_t> t_circulate_r(std::size_t t);
	std::vector<std::size_t> t_circulate_t(std::size_t t);
	std::vector<std::size_t> region_sites(std::size_t r);
	std::vector<std::size_t> neighbour_regions(std::size_t r);
	std::vector<std::size_t> neighbour_triangle_centers(std::size_t r);
};

float randFloat();

std::vector<vec3> generateFibonacciSphere(int N, float jitter = 0);

TriangleMesh generateDelanuaySphere(std::vector<vec3>* verticles);
void addSouthPoleToMesh(std::size_t southPoleId, Delaunator * delanuay, std::vector<vec3> * p);


