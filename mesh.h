#pragma once

//https://ru.wikipedia.org/wiki/%D0%9F%D1%80%D0%B0%D0%B2%D0%B8%D0%BB%D1%8C%D0%BD%D1%8B%D0%B9_%D0%B8%D0%BA%D0%BE%D1%81%D0%B0%D1%8D%D0%B4%D1%80

#include <vector>
#include <list>
#include "../Content/PlanetGen/MathStructures.h"

using namespace DirectX;

class Mesh
{
public:
	std::vector<XMFLOAT3> vertexes;
	std::list<std::vector<unsigned short>> triangles;

	static Mesh GenerateIsocahedronMesh();

    void generateSubdivisions(int n);
	void normalizeVertexes(float sphereRadius);

	int vertexExist(XMFLOAT3 vertex);
	int triangleExist(std::vector<unsigned short> triangle);
};
