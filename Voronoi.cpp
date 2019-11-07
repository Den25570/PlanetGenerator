#include "Voronoi.h"

Voronoi::Voronoi(Triangulation* triangulation)
{
	for (auto point : triangulation->points)
	{
		auto angle_list = new std::vector<DEdge*>;
		for (auto e : point.edges)
			angle_list->push_back(e);

		auto sorted_e = sortEdgesByAngle(angle_list, &point);		
		AddSite(sorted_e, point.position);

		free(angle_list);
	}
}

void Voronoi::AddSite(std::list<DEdge*>* edges_list, Vector3 pos)
{
	auto site = new VoronoiSite();
	std::list<HEdge*> added_edges;

	//Создание полу-ребра
	for (auto e : *edges_list)
	{
		Vector3 middle_p = (e->nodes[0]->position + e->nodes[1]->position) * (1 / 2);

		Node* new_node = nodeExist(middle_p);
		if (new_node == NULL)
		{
			new_node = new Node(middle_p);
			nodes.push_back(new_node);		
		}
		added_edges.push_back(new HEdge(new_node, site));
	}

	//Связывание полу-ребер
	auto prev_edge = added_edges.end()--;
	for (auto it = added_edges.begin(); it != added_edges.end(); it++)
	{
		(*prev_edge)->next = (*it);
		(*prev_edge)->dest->edges.push_back((*it));

		auto opposite_edge = (*it)->dest->findConnectedNode((*prev_edge)->dest);
		if (opposite_edge != NULL)
		{
			(*it)->opposite = opposite_edge;
			opposite_edge = (*it)->opposite;
		}
	}
}

Node* Voronoi::nodeExist(Vector3 pos)
{
	for (auto n : nodes)
		if (n->position == pos)
			return n;
	return NULL;
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
	float max = std::numeric_limits<float>::max();
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


HEdge* Node::findConnectedNode(Node* n)
{
	for (auto e : edges)
		if (e->dest == n)
			return e;
	return NULL;
}
