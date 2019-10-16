#include "pch.h"
#include "mesh.h"
#include <queue>

using namespace DirectX;

Mesh Mesh::GenerateIsocahedronMesh()
{
	float phi = (1.0 + sqrt(5.0)) / 2.0;
	float du = 1.0 / sqrt(phi * phi + 1.0);
	float dv = phi * du;

	Mesh icosahedron;

	icosahedron.vertexes =
	{
		 Vector3(0, +dv, +du),
		 Vector3(0, +dv, -du),
		 Vector3(0, -dv, +du),
		 Vector3(0, -dv, -du),

		 Vector3(+du, 0, +dv),
		 Vector3(-du, 0, +dv),
		 Vector3(+du, 0, -dv),
		 Vector3(-du, 0, -dv),

		 Vector3(+dv, +du, 0),
		 Vector3(+dv, -du, 0),
		 Vector3(-dv, +du, 0),
		 Vector3(-dv, -du, 0),
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

//Рекурсивное подразделение треугольников исокаэдра
void Mesh::generateSubdivisions_recursive(int n)
{
	for (int i = 0; i < n; i++)
	{
		int j = 0;
		int end = triangles.size();

		for (auto it = triangles.begin(); j < end; it++, j++)
		{
			Vector3 v0 = vertexes[(*it)[0]];
			Vector3 v1 =  vertexes[(*it)[1]];
			Vector3 v2 =  vertexes[(*it)[2]];

			//Создание новых вершин, если уже существует, то использовать имеющуюся
			Vector3 v3 = Vector3((v0.x + v1.x)* 0.5f, (v0.y + v1.y)* 0.5f, (v0.z + v1.z)* 0.5f);
			Vector3 v4 = Vector3((v1.x + v2.x)* 0.5f, (v1.y + v2.y)* 0.5f, (v1.z + v2.z)* 0.5f);
			Vector3 v5 = Vector3((v0.x + v2.x)* 0.5f, (v0.y + v2.y)* 0.5f, (v0.z + v2.z)* 0.5f);

			int vu3 = checkAndAddVertex(&vertexes, v3);
			int vu4 = checkAndAddVertex(&vertexes, v4);
			int vu5 = checkAndAddVertex(&vertexes, v5);

			//Добавление новых треугольников
			triangles.push_back({ static_cast<unsigned short>(vu4), (*it)[2], static_cast<unsigned short>(vu5) });
		    triangles.push_back({ static_cast<unsigned short>(vu3), (*it)[1], static_cast<unsigned short>(vu4) });
		    triangles.push_back({ static_cast<unsigned short>(vu3), static_cast<unsigned short>(vu4), static_cast<unsigned short>(vu5) });
			*it = std::vector<unsigned short>{ (*it)[0], static_cast<unsigned short>(vu3), static_cast<unsigned short>(vu5) };
		}
	}
}

// Нерекурсивное подразделение треугольников исокаэдра n > 1
void Mesh::generateSubdivisions_nonRecursive(int n)
{
	int j = 0;
	int size = triangles.size();

	for (auto it = triangles.begin(); j < size; it++, j++)
	{
		std::queue<unsigned short> queue;
		std::vector<Vector3> bottom_vertexes;
		std::vector<Vector3> bottom_vertexes_rev;

		float Len = Vector3::length(vertexes[(*it)[0]] - vertexes[(*it)[1]]) / n;

		//Вектор дополнительных точек треугольника
		bottom_vertexes.push_back(vertexes[(*it)[1]]);
		for (int i = 1; i < n; i++)
			bottom_vertexes.push_back(Vector3::calcVector(vertexes[(*it)[1]], vertexes[(*it)[2]], Len*i));
		bottom_vertexes.push_back(vertexes[(*it)[2]]);

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

				Vector3 v0 = Vector3::calcVector(vertexes[index], bottom_vertexes[j], Len);
				Vector3 v1 = Vector3::calcVector(vertexes[index], bottom_vertexes_rev[i - j], Len);

				int vu0 = checkAndAddVertex(&vertexes, v0);
				int vu1 = checkAndAddVertex(&vertexes, v1);

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

int checkAndAddVertex(std::vector<Vector3> *vertexes, Vector3 vertex)
{
	int vu = vertexExist(vertexes, vertex);
	if (vu == -1)
	{
		vu = vertexes->size();
		vertexes->push_back(vertex);
	}
	return vu;
}

int vertexExist(std::vector<Vector3> *vertexes, Vector3 vertex)
{
	for (int i = 0; i < vertexes->size(); i++)
	{
		if (compareF((*vertexes)[i].x, vertex.x) && compareF((*vertexes)[i].y, vertex.y) && compareF((*vertexes)[i].z, vertex.z))
			return i;
	}
	return -1;
}

int triangleExist(std::list<std::vector<unsigned short>> *triangles, std::vector<unsigned short> triangle)
{
	int i = 0;
	for (auto it = triangles->cbegin(); it != triangles->cend(); it++, i++)
	{
		if ((*it)[0] == triangle[0] && (*it)[1] == triangle[1] && (*it)[2] == triangle[2])
			return i;
	}
	return -1;
}