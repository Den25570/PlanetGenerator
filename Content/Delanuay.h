#pragma once
#pragma once
#include "Math.h"
#include <mesh.h>

class Triangulation
{
public:
	std::vector<DNode> points;
	std::vector<DTriangle> triangles;

    void generateTriangulation2(std::vector<Vector3> _points);
	
private:
    void InitializePoints(std::vector<Vector3> _points);
	void SortPointsByX(std::vector<DNode>* nodes, int start, int end);
	std::vector<int> getObsEdges(std::vector<DEdge *> *minConvexHull, std::stack<DEdge *> *obsEdges, int index, int last_add_index);
	std::vector<std::vector<DNode*>>* DevideNodes();
};

class DEdge
{
public:
	DNode *nodes[2];
	std::vector<DTriangle*> triangles;

	inline DEdge(DNode* n0, DNode* n1)
	{
		nodes[0] = n0;
		nodes[1] = n1;
	};

	int getTriangleIndex(DTriangle* t);
};

class DTriangle
{
public:
	DEdge *edges[3];

	Vector3 circle_pos;
	float radius_c;

	inline void calcCircum();

	inline DTriangle(DEdge* e0, DEdge* e1, DEdge* e2)
	{
		e0->triangles.push_back(this); edges[0] = e0;
		e1->triangles.push_back(this); edges[1] = e1;
		e2->triangles.push_back(this); edges[2] = e2;
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