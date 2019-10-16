#pragma once

#include <vector>
#include <list>
#include "../Content/Math.h"

using namespace DirectX;

class Mesh
{
public:
	std::vector<Vector3> vertexes;
	std::list<std::vector<unsigned short>> triangles;

	static Mesh GenerateIsocahedronMesh();

    void generateSubdivisions_recursive(int n);
	void generateSubdivisions_nonRecursive(int n);
	void normalizeVertexes(float sphereRadius);
	
};

int checkAndAddVertex(std::vector<Vector3> *vertexes, Vector3 vertex);
int vertexExist(std::vector<Vector3> *vertexes, Vector3 vertex);
int triangleExist(std::list<std::vector<unsigned short>> *triangles, std::vector<unsigned short> triangle);
