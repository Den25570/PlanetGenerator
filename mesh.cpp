#include "pch.h"
#include "mesh.h"

using namespace DirectX;

Mesh Mesh::GenerateIsocahedronMesh()
{
	float phi = (1.0 + sqrt(5.0)) / 2.0;
	float du = 1.0 / sqrt(phi * phi + 1.0);
	float dv = phi * du;

	Mesh icosahedron;

	icosahedron.vertexes =
	{
		 XMFLOAT3(0, +dv, +du),
		 XMFLOAT3(0, +dv, -du),
		 XMFLOAT3(0, -dv, +du),
		 XMFLOAT3(0, -dv, -du),

		 XMFLOAT3(+du, 0, +dv),
		 XMFLOAT3(-du, 0, +dv),
		 XMFLOAT3(+du, 0, -dv),
		 XMFLOAT3(-du, 0, -dv),

		 XMFLOAT3(+dv, +du, 0),
		 XMFLOAT3(+dv, -du, 0),
		 XMFLOAT3(-dv, +du, 0),
		 XMFLOAT3(-dv, -du, 0),
	};

	icosahedron.triangles =
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

	return icosahedron;
}

void Mesh::generateSubdivisions(int n)
{
	for (int i = 0; i < n; i++)
	{
		int j = 0;
		int end = triangles.size();

		for (auto it = triangles.begin(); j < end; it++, j++)
		{
			XMFLOAT3 v0 = vertexes[(*it)[0]];
			XMFLOAT3 v1 =  vertexes[(*it)[1]];
			XMFLOAT3 v2 =  vertexes[(*it)[2]];

			XMFLOAT3 v3 = XMFLOAT3((v0.x + v1.x)* 0.5f, (v0.y + v1.y)* 0.5f, (v0.z + v1.z)* 0.5f);
			XMFLOAT3 v4 = XMFLOAT3((v1.x + v2.x)* 0.5f, (v1.y + v2.y)* 0.5f, (v1.z + v2.z)* 0.5f);
			XMFLOAT3 v5 = XMFLOAT3((v0.x + v2.x)* 0.5f, (v0.y + v2.y)* 0.5f, (v0.z + v2.z)* 0.5f);

		     short ver3 = vertexExist(v3);
			 short ver4 = vertexExist(v4);
			 short ver5 = vertexExist(v5);

			if (ver3 == -1)
			{
				ver3 =  vertexes.size();
				 vertexes.push_back(v3);
			}
			if (ver4 == -1)
			{
				ver4 =  vertexes.size();
				 vertexes.push_back(v4);
			}
			if (ver5 == -1)
			{
				ver5 =  vertexes.size();
				 vertexes.push_back(v5);
			}

			triangles.push_back({ static_cast<unsigned short>(ver4), (*it)[2], static_cast<unsigned short>(ver5) });
		    triangles.push_back({ static_cast<unsigned short>(ver3), (*it)[1], static_cast<unsigned short>(ver4) });			 
		    triangles.push_back({ static_cast<unsigned short>(ver3), static_cast<unsigned short>(ver4), static_cast<unsigned short>(ver5) });	
			*it = std::vector<unsigned short>{ (*it)[0], static_cast<unsigned short>(ver3), static_cast<unsigned short>(ver5) };
		}
	}
}

void Mesh::normalizeVertexes(float sphereRadius)
{
	for (auto it = vertexes.begin(); it != vertexes.end(); it++)
	{
		float length = sqrtf(it->x * it->x + it->y * it->y  + it->z * it->z);
		it->x *= sphereRadius / length;
		it->y *= sphereRadius / length;
		it->z *= sphereRadius / length;
	}
}

int Mesh::vertexExist(XMFLOAT3 vertex)
{
	for (int i = 0; i < vertexes.size(); i++)
	{
		if (compareF(vertexes[i].x, vertex.x) && compareF(vertexes[i].y, vertex.y) && compareF(vertexes[i].z, vertex.z))
			return i;
	}
	return -1;
}

int Mesh::triangleExist(std::vector<unsigned short> triangle)
{
	int i = 0;
	for (auto it = triangles.cbegin(); it != triangles.cend(); it++, i++)
	{
		if ((*it)[0] == triangle[0] && (*it)[1] == triangle[1] && (*it)[2] == triangle[2])
			return i;
	}
	return -1;
}