#pragma once

#include <vector>
#include <list>
#include "../Content/PlanetGen/Math.h"

using namespace DirectX;

class Mesh
{
public:
	std::vector<XMFLOAT3> vertexes;
	std::list<std::vector<unsigned short>> triangles;

	static Mesh GenerateIsocahedronMesh();

    void generateSubdivisions(int n);
	void generateSubdivisions_v2(int n);
	void normalizeVertexes(float sphereRadius);

	int vertexExist(XMFLOAT3 vertex);
	int triangleExist(std::vector<unsigned short> triangle);
};
