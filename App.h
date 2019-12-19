#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_opengl3.h"
#include <stdio.h>

#ifndef GRAPHICS_H
#define GRAPHICS_H
	#define GLEW_STATIC
	#include <gl/glew.h>
	#include  <gl/glfw3.h>
#endif

#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

#include <ctime>

#include "Camera.h"
#include "Shader.hpp"
#include "VAO.hpp"
#include "Texture.hpp"

#include "Planet.hpp"
#include "StructToFlat.hpp"
#include "SkyBox.hpp"

int main();

void drawFrameBuffer(VAO &frame_quad_VAO, Shader &blurFrameShader, unsigned int  colorBuffers[2], unsigned int  pingpongColorbuffers[2], bool horizontal = true);
GLFWwindow *  InitWindow(int * settings);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void UpdateCamera();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);