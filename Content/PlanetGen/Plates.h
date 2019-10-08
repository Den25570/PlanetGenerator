#pragma once

#include <list>
#include "PlanetData.h"

class Plate
{
public:
	std::list<PlanetSite*> sites;
	std::list<Voronoi::Edge*> boundaries;

	VectorM move_speed;

	//Color display_color;
};