#pragma once

#include "Object.hpp"

class SkyBox : public Object
{
public:
	SkyBox(Shader* shader);
	void draw(glm::mat4 * View, glm::mat4 * Projection);
private:
	unsigned int cubemapTexture;
};