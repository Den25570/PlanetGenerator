#pragma once
#pragma once
#include "Math.h"
#include <mesh.h>

class Triangulation
{
public:
	std::vector<DNode> points;
	std::list<DTriangle*> triangles;

    void generateTriangulation2(std::vector<Vector3> _points);	
private:
    void InitializePoints(std::vector<Vector3> _points);
	void SortPointsByX(int start, int end);
	void getObsEdges(std::vector<DEdge *> *minConvexHull, std::stack<DEdge *> *obsEdges, int index, int last_add_index);
	std::vector<std::vector<DNode*>>* DevideNodes();
	DTriangle* AddNewTriangle(DEdge * e1, DEdge * e2, DEdge * e3);
};

class DEdge
{
public:
	DNode *nodes[2];
	std::vector<DTriangle*> triangles;

	inline DEdge(DNode* n0, DNode* n1)
	{
		nodes[0] = n0; nodes[1] = n1;
	};

	int getTriangleIndex(DTriangle* t);
};

class DTriangle
{
public:
	DEdge *edges[3];

	//Precalculated circle data
	Vector3 circle_pos;
	float radius_c;
	inline void calcCircum();

	inline DTriangle(DEdge* e0, DEdge* e1, DEdge* e2)
	{
		e0->triangles.push_back(this); edges[0] = e0; e1->triangles.push_back(this); edges[1] = e1; e2->triangles.push_back(this); edges[2] = e2;
	};

    ~DTriangle()
	{
		for (int i = 0; i < 3; i++)
			edges[i]->triangles.erase(edges[i]->triangles.begin() + edges[i]->getTriangleIndex(this));
	}
};

class DNode
{
public:
	Vector3 position;
	std::list<DEdge*> edges;

	inline DNode(Vector3 _position)
	{
		position = _position;
	};
};