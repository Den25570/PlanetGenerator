#pragma once
#include "Delanuay.h"

class Node;
class HEdge;
class VoronoiSite;

class Voronoi
{
public:
	std::vector<Node*> nodes;
	std::vector<VoronoiSite*> sites;

	Voronoi(Triangulation* triangulation);
private:
	std::list<DEdge*>* sortEdgesByAngle(std::vector<DEdge*> * edges, DNode* point);
};

class Node
{
public:
	Vector3 position;
	std::vector<HEdge*> edges;
};

class HEdge
{
public:
	Node* dest;
	HEdge* next;
	HEdge* opposite;
	VoronoiSite* site;

	HEdge();
};

class VoronoiSite
{
public:
	HEdge* start_edge;
	Vector3 position;

	VoronoiSite(std::list<DEdge*>* edges_list, DNode* pos);

	std::list<VoronoiSite*> getNeighbours();
	std::list<HEdge*> getEdges();
};