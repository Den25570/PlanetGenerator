#pragma once
#pragma once
#include "Math.h"
#include <mesh.h>

class Triangulation
{
public:
	std::vector<DNode> points;
	std::vector<DTriangle> triangles;

	static void generateTriangulation2(Triangulation *triangulation, std::vector<Vector3> _points);
private:
    void InitializePoints(std::vector<Vector3> _points);
	void SortPointsByX(std::vector<DNode>* nodes, int start, int end);

	std::vector<std::vector<DNode*>>* DevideNodes();
};

class DEdge
{
public:
	DNode *nodes[2];
	std::vector<DTriangle*> triangles[2];

	inline DEdge(DNode* n0, DNode* n1)
	{
		nodes[0] = n0;
		nodes[1] = n1;
	};
};

class DTriangle
{
public:
	DEdge *edges[3];

	float x_c;
	float y_c;
	float radius_sqr;

	inline void calcCircum();

	inline DTriangle(DEdge* e0, DEdge* e1, DEdge* e2)
	{
		e0->triangles->push_back(this); edges[0] = e0;
		e1->triangles->push_back(this); edges[1] = e1;
		e2->triangles->push_back(this); edges[2] = e2;
	};
};

class DNode
{
public:
	Vector3 position;
	inline DNode(Vector3 _position)
	{
		position = _position;
	};
};