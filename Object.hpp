#pragma once

#include "VAO.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

//virtual super class for all scene objects
class Object {
public:
	bool initialized = false;

	VAO vertex_array_object;
	Shader* shader = nullptr;

	Object() { initialized = true; };
	~Object() {};
	virtual void draw() {};
protected:
	int draw_mode;
	int draw_mode_side;
	int polygon_mode;
};
