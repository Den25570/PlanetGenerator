#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <gl/glew.h>
#include <gl/glfw3.h>
#endif

#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

class Shader
{
public:
	//ID
	GLuint Program;
	//Read and compile shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

	//Connects shader
	void Use();

	void setBool(const std::string &name, bool value);
	void setInt(const std::string &name, int value);
	void setFloat(const std::string &name, float value);
	void setVec2(const std::string &name, const glm::vec2 &value); 
	void setVec2(const std::string &name, float x, float y);
	void setVec3(const std::string &name, const glm::vec3 &value);
	void setVec3(const std::string &name, float x, float y, float z);
	void setVec4(const std::string &name, const glm::vec4 &value) ;
	void setVec4(const std::string &name, float x, float y, float z, float w);
	void setMat2(const std::string &name, const glm::mat2 &mat);
	void setMat3(const std::string &name, const glm::mat3 &mat);
	void setMat4(const std::string &name, const glm::mat4 &mat);
private:
	//Загрузка шейдеров
	std::string ReadShaderCode(const GLchar * path);
	GLint CompileShader(std::string shaderCode, const GLchar * path, GLuint ID);

	std::vector<GLuint> uniforms;
};