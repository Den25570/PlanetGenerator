#include "pch.h"
#include "mesh.h"

using namespace DirectX;

Mesh Mesh::GenerateIsocahedronMesh()
{
	float phi = (1.0 + sqrt(5.0)) / 2.0;
	float du = 1.0 / sqrt(phi * phi + 1.0);
	float dv = phi * du;

	Mesh mesh;

	mesh.verticles =
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

	mesh.triangles =
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

	return mesh;
}

void Mesh::generateSubdivisions(int n, Mesh* startMesh)
{
	for (int i = 0; i < n; i++)
	{
		for (auto it = startMesh->triangles.begin(); it != startMesh->triangles.end(); ++it)
		{
			XMFLOAT3 v0 = startMesh->verticles[(*it)[0]];
			XMFLOAT3 v1 = startMesh->verticles[(*it)[1]];
			XMFLOAT3 v2 = startMesh->verticles[(*it)[2]];

			XMFLOAT3 v3 = XMFLOAT3((v0.x + v1.x)* 0.5f, (v0.y + v1.y)* 0.5f, (v0.z + v1.z)* 0.5f);
			XMFLOAT3 v4 = XMFLOAT3((v1.x + v2.x)* 0.5f, (v1.y + v2.y)* 0.5f, (v1.z + v2.z)* 0.5f);
			XMFLOAT3 v5 = XMFLOAT3((v0.x + v2.x)* 0.5f, (v0.y + v2.y)* 0.5f, (v0.z + v1.z)* 0.5f);

			unsigned short ver3 = startMesh->verticles.size();
			unsigned short ver4 = startMesh->verticles.size() + 1;
			unsigned short ver5 = startMesh->verticles.size() + 2;

			startMesh->verticles.push_back(v3);
			startMesh->verticles.push_back(v4);
			startMesh->verticles.push_back(v5);

			startMesh->triangles.push_back({ (*it)[0], ver3, ver5 });
			startMesh->triangles.push_back({ ver3, (*it)[1], ver4 });
			startMesh->triangles.push_back({ ver4, (*it)[2], ver5 });
			startMesh->triangles.push_back({ ver3, ver4,     ver5 });
		}
	}
}