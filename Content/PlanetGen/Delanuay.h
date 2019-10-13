#pragma once
#pragma once
#include "Math.h"
#include <mesh.h>

class Triangulation
{
	std::vector<DelanuayPoint> points;
	std::vector<DelanuayEdge> edges;
};

class DelanuayPoint
{
	Vector3 position;
	std::vector<DelanuayEdge> edges;
};

class DelanuayEdge
{
	DelanuayPoint* start;
	DelanuayPoint* end;
};