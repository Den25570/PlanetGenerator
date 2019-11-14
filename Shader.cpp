#include "Shader.hpp"

#include <vector>

Shader::Shader(const GLchar * vertexPath, const GLchar * fragmentPath) {

	// Создаем шейдеры
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Загружаем код шейдеров
	std::string VertexShaderCode = ReadShaderCode(vertexPath);
	std::string FragmentShaderCode = ReadShaderCode(fragmentPath);

	//Компилируем шейдеры
	CompileShader(VertexShaderCode, vertexPath, VertexShaderID);
	CompileShader(FragmentShaderCode, fragmentPath, FragmentShaderID);

	// Создаем шейдерную программу и привязываем шейдеры к ней
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Проверяем шейдерную программу
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

    Program = ProgramID;
}

std::string Shader::ReadShaderCode(const GLchar * path)
{
	std::string ShaderCode;
	std::ifstream ShaderStream(path, std::ios::in);
	if (ShaderStream.is_open())
	{
		std::stringstream sstr;
		sstr << ShaderStream.rdbuf();
		ShaderCode = sstr.str();
		ShaderStream.close();
	}
	return ShaderCode;
}

int Shader::CompileShader(std::string shaderCode, const GLchar * path, GLuint ID)
{
	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Компилируем шейдер
	std::cout << "Компиляция шейдера: " << path << std::endl;
	char const * VertexSourcePointer = shaderCode.c_str();
	glShaderSource(ID, 1, &VertexSourcePointer, NULL);
	glCompileShader(ID);

	// Выполняем проверку шейдера
	glGetShaderiv(ID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		std::cout << &VertexShaderErrorMessage[0] << std::endl;
	}
	return Result;
}

void Shader::Use() { glUseProgram(this->Program); }