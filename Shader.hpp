#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <gl/glew.h>

class Shader
{
public:
	//ID
	GLuint Program;
	//Read and compile shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	//Connects shader
	void Use();
private:
	//Загрузка шейдеров
	std::string ReadShaderCode(const GLchar * path);
	int CompileShader(std::string shaderCode, const GLchar * path, GLuint ID);
};