#include "pch.h"
#include "..\Content\Voronoi.h"

Voronoi::Voronoi(Triangulation* triangulation)
{
	for (auto point : triangulation->points)
	{
		std::vector<DEdge*> angle_list;
		std::copy(&(point.edges).begin(), &(point.edges).end(), angle_list);

		auto sorted_e = sortEdgesByAngle(&angle_list, &point);
		
	}
}

void Voronoi::AddSite(std::list<DEdge*>* edges_list, Vector3 pos)
{
	Node* prev_node = NULL;
	bool prev_node_existed = false;
	std::list<Node*> added_nodes;

	for (auto e : *edges_list)
	{
		Vector3 middle_p = (e->nodes[0]->position + e->nodes[1]->position) * (1/2);

		Node* new_node = nodeExist(middle_p);
		if (new_node != NULL)
		{	
			if (prev_node != NULL && prev_node_existed)
			{

			}
			prev_node_existed = true;
		}		
		else 
		{
			prev_node_existed = false;
			new_node = new Node(middle_p);
			added_nodes.push_back(new_node);
		}
			

		prev_node = new_node;
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


bool Node::findConnectedNode(Node* n)
{
	for (auto e : edges)
		if (e->dest == n)
			return true;
	return false;
}
