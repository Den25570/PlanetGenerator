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
	Node* nodeExist(Vector3 pos);
	HEdge* halfEdgeExist(Node* n1 , Node* n2);
	void AddSite(std::list<DEdge*>* edges_list, Vector3 pos);
};

class Node
{
public:
	Vector3 position;
	std::vector<HEdge*> edges;

	Node(Vector3 pos) { position = pos; }

	bool findConnectedNode(Node* n);
};

class HEdge
{
public:
	Node* dest;
	HEdge* next;
	HEdge* opposite;
	VoronoiSite* site;

	HEdge(Node* _dest, VoronoiSite* _site) { dest = _dest; site = _site;}
};

class VoronoiSite
{
public:
	HEdge* start_edge;
	Vector3 position;

	VoronoiSite(void) {};

	std::list<VoronoiSite*> getNeighbours();
	std::list<HEdge*> getEdges();
};