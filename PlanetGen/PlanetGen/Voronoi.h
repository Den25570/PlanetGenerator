#pragma once

#include "MathStructures.h"
#include <list>

namespace Voronoi
{
	class Voronoi
	{
		std::list<Point*> edge_points;
		std::list<Edge*> edges;
		std::list<Face*> faces;
	};

	class Point
	{
		Vector3 position;
		std::list<Edge*> edges;
		Edge* out_edge;
	};

	class Face
	{
		std::list<Face*> neighbour_faces;
		std::list<Edge*> edges;
		Edge* start_edge;
		Vector3 center_point;
	};

	class Edge
	{
		Point* start_point;
		Edge* next;
		Edge* prev;
		Edge* opposite;
	};
}

