#include "pch.h"
#include "..\Content\Voronoi.h"

Voronoi::Voronoi(Triangulation* triangulation)
{
	for (auto point : triangulation->points)
	{
		std::vector<DEdge*> angle_list;
		std::copy(&(point.edges).begin(), &(point.edges).end(), angle_list);

		nodes.push_back();

		auto sorted_e = sortEdgesByAngle(&angle_list, &point);

	}
	
}

std::list<DEdge*>* Voronoi::sortEdgesByAngle(std::vector<DEdge*> * edges, DNode* point)
{
	auto res = new std::list<DEdge*>;
	std::vector<float> angle_list(edges->size());

	for (auto e : *edges)
	{
		Vector3 p = e->nodes[!e->getNodeIndex(point)]->position - point->position;
		float angle = p.getPolarAngle();
		angle_list.push_back(angle);
	}
	float max = D3D12_FLOAT32_MAX;
	while (res->size() < edges->size())
	{
		int max_i = 0;
		for (int i = 1; i < angle_list.size(); i++)
			if (angle_list[i] > angle_list[max_i] && angle_list[i] < max)
				max_i = i;
		res->push_back((*edges)[max_i]);
		max = angle_list[max_i];
	}
	return res;
}

VoronoiSite::VoronoiSite(std::list<DEdge*>* edges_list, DNode* pos)
{
	for (auto e : *edges_list)
	{

	}
}

