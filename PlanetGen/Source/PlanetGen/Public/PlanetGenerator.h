// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <cmath>
#include "Planet.h"
#include "WorldGeometry.h"

/**
 * 
 */
class PLANETGEN_API PlanetGenerator
{
public:
	void generatePlanetAsynchronous();
	void generatePlanetPartition(std::vector<Tile> tiles/*, action*/);
	void generatePlanetTopology(Mesh mesh/*, action*/);
	void generatePlanetBiomes(std::vector<Tile> tiles, float planetRadius/*, action*/);
	float adjustRange(float value, float oldMin, float oldMax, float newMin, float newMax);
};