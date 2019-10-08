#pragma once
#include "MathStructures.h"
#include <list>

class Triangulation2
{
	std::list<DelanuayPoint> points;
	std::list<DelanuayEdge> points;
};

class DelanuayPoint
{
	Vector3 position;
	std::list<DelanuayEdge> edges;
};

class DelanuayEdge
{
	DelanuayPoint* start;
	DelanuayPoint* end;
};