// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Engine/StaticMesh.h"
#include <cmath>
#include <vector>

/**
 * 
 */
class PLANETGEN_API WorldGeometry
{
public:
	static Icosahedron* generateIcosahedron();
	int getEdgeOppositeFaceIndex(Edge edge, int faceIndex);
	int findNextFaceIndex(UStaticMesh mesh, int nodeIndex, int faceIndex);

private:
	WorldGeometry() {};

};

struct Icosahedron
{
	std::vector<Face*> faces;
	std::vector<Edge*> edges;
	std::vector<Node*> nodes;

	inline Icosahedron(std::vector<Face*> f, std::vector<Edge*> e, std::vector<Node*> n)
	{
		nodes = n;
		edges = e;
		faces = f;
	}
};

struct Node
{
	FVector* position;
	std::vector<int>* edges;
	std::vector<int>* faces;

	inline Node(FVector* p)
	{
		position = p;
	}
	inline Node(FVector* p, std::vector<int>* e, std::vector<int>* f)
	{
		position = p;
		edges = e;
		faces = f;
	}
};

struct Edge
{
	std::vector<int>* faces;
	std::vector<int>* nodes;

	inline Edge(std::vector<int>* n, std::vector<int>* f)
	{
		nodes = n;
		faces = f;
	}
};

struct Face
{
	std::vector<int>* edges;
	std::vector<int>* nodes;
	std::vector<Tile*> children;

	Sphere* bounding_sphere;
	

	inline Face(std::vector<int>* n, std::vector<int>* e)
	{
		edges = e;
		nodes = n;
	}
};

struct Sphere
{
	FVector center;
	float radius;

	inline Sphere(FVector _center, float _radius)
	{
		center = _center;
		radius = _radius;
	}
};

class Tile
{
public:
	float elevation;
	float latitude;
	float temperature;
	float moisture = moisture;

	FVector position;

	Sphere* bounding_sphere;

	std::string biome;
};

struct Mesh
{
	std::vector<Face*> faces;
	std::vector<Edge*> edges;
	std::vector<Node*> nodes;
};
