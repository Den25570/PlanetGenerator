#pragma once

#include <vector>

using namespace DirectX;

class Mesh
{
public:
	std::vector<XMFLOAT3> verticles;
	std::vector<std::vector<unsigned short>> triangles;

	static Mesh GenerateIsocahedronMesh();
    void generateSubdivisions(int n, Mesh* startMesh);
};
