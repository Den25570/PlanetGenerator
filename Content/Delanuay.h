#pragma once
#pragma once
#include "Math.h"
#include <mesh.h>

class Triangulation
{
public:
	std::vector<DelanuayPoint> points;
	std::vector<DelanuayEdge> edges;

	static void generateTriangulation_2D(Triangulation *triangulation, std::vector<Vector3> _points);
private:
    void InitializePoints(std::vector<Vector3> _points);
	void SortPointsByX(int start, int end);
};

class DelanuayPoint
{
public:
	Vector3 position;
	std::vector<DelanuayEdge*> edges;

	inline DelanuayPoint(void) {};
	inline DelanuayPoint(Vector3 _position)
	{
		position = _position;
	};
};

class DelanuayEdge
{
public:
	DelanuayPoint* start;
	DelanuayPoint* end;

	inline DelanuayEdge(void) {};
	inline DelanuayEdge(DelanuayPoint* p0, DelanuayPoint* p1) 
	{
		start = p0;
		end = p1;
	};
};