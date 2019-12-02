#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>

#include <vector>
#include "Shader.hpp"

class VAO {
public:
	VAO() {};
	VAO(Shader * shader, std::vector<int> attributes_size, std::vector<GLfloat> * vertices, std::vector<GLuint> * indices = NULL) {	
		this->shader = shader;
		vertices_size = vertices->size();
		indices_size = indices != NULL ? indices->size() : 0;
		
		initializeBuffers(attributes_size, vertices, indices);
	}

	void use(int polygon_mode_side, int polygon_mode);
	void draw(int draw_mode);
private:
	GLuint VAO_index;
	GLuint VBO_index;
	GLuint EBO_index;;

	GLuint vertices_size;
	GLuint indices_size;

	Shader * shader;

	void initializeBuffers(std::vector<int> attributes_size, std::vector<GLfloat> * vertices, std::vector<GLuint> * indices);
};
