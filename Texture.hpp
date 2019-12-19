#pragma once

#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <gl/glew.h>
#include <gl/glfw3.h>
#endif


#include <stb_image.h>
#include <vector>
#include <iostream>

using std::vector;

unsigned int loadCubemap(vector<std::string> faces);
unsigned int generatePlanetTexture();