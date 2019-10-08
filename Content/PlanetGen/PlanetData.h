#pragma once

#include "Voronoi.h"
#include "Plates.h"

class Graph
{
public:
	Voronoi::Voronoi voronoi;
	std::list<PlanetSite*> sites;
	std::list<Plate*> plates;
	std::list<River*> rivers;

	void GenerateHeightMap();
	void GeneratePlates();
};

class PlanetSite
{
	Voronoi::Face* face;
	Biom biom;
	Plate* plate;
};

enum Biom
{
	// ...
};

class River
{
	std::list<Voronoi::Edge*> flow_edges;
};