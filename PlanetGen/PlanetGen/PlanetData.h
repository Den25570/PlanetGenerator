#pragma once

#include "Voronoi.h"

class Graph
{
	Voronoi::Voronoi voronoi;
	std::list<PlanetSite*> sites;
	std::list<Plate*> plates;
	std::list<River*> rivers;
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

class Plate
{
	std::list<PlanetSite*> sites;
	std::list<Voronoi::Edge*> boundaries;

	VectorM move_speed;

	//Color display_color;
};

class River
{
	std::list<Voronoi::Edge*> flow_edges;
};