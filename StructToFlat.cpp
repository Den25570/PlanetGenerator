#include "StructToFlat.hpp"

std::vector<float> vec3ToFlat(std::vector<glm::vec3> orig) {
	std::vector<float> res;
	for (auto p : orig)
	{
		res.push_back(p.x);
		res.push_back(p.y);
		res.push_back(p.z);
	}
	return res;
}

std::vector<double> vec2ToDouble(std::vector<glm::vec2> * points) {
	std::vector<double> res;
	for (auto p : *points)
	{
		res.push_back((double)p.x);
		res.push_back((double)p.y);
	}
	return res;
}

void meshToFlat(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const TriangleMesh * planetMesh)
{
	vertices = std::vector<GLfloat>(planetMesh->points.size() * 5);
	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		(vertices)[i * 5 + 0] = planetMesh->points[i].x;
		(vertices)[i * 5 + 1] = planetMesh->points[i].y;
		(vertices)[i * 5 + 2] = planetMesh->points[i].z;
		(vertices)[i * 5 + 3] = 0;
		(vertices)[i * 5 + 4] = 0;
	}
	indices = std::vector<GLuint>(planetMesh->triangles.size());
	for (int i = 0; i < planetMesh->triangles.size(); i++)
		(indices)[i] = planetMesh->triangles[i];
}

void quadToFlat(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const QuadGeometry * planetMesh)
{
	vertices = std::vector<GLfloat>(planetMesh->points.size() * 2);
	for (std::size_t i = 0; i < planetMesh->points.size() / 3; i++)
	{
		(vertices)[i * 6 + 0] = planetMesh->points[i * 3 + 0];
		(vertices)[i * 6 + 1] = planetMesh->points[i * 3 + 1];
		(vertices)[i * 6 + 2] = planetMesh->points[i * 3 + 2];

		(vertices)[i * 6 + 3] = planetMesh->tem_mois[i * 3 + 0];
		(vertices)[i * 6 + 4] = planetMesh->tem_mois[i * 3 + 1];
		(vertices)[i * 6 + 5] = planetMesh->tem_mois[i * 3 + 2];
	}
	indices = std::vector<GLuint>(planetMesh->indices.size());
	for (int i = 0; i < planetMesh->indices.size(); i++)
		(indices)[i] = planetMesh->indices[i];
}

void voronoiToFlat(std::vector<GLfloat> &vertices, const Voronoi * planetMesh)
{
	vertices = std::vector<GLfloat>(planetMesh->points.size() * 6);
	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		vertices[i * 6 + 0] = planetMesh->points[i].x;
		vertices[i * 6 + 1] = planetMesh->points[i].y;
		vertices[i * 6 + 2] = planetMesh->points[i].z;
	
		vertices[i * 6 + 3] = planetMesh->tm[i].x;
		vertices[i * 6 + 4] = planetMesh->tm[i].y;    
		vertices[i * 6 + 5] = planetMesh->tm[i].z;
	}
}
