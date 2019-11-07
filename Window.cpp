#include <iostream>
#include <fstream>
#include <sstream>
#include "Window.h"

GLFWwindow * InitWindow(int * settings) {
	if (!glfwInit())
	{
		std::cout << "GLW can't be initialized." << std::endl;
		return NULL;
	}

	//Инициализация окна
	glfwWindowHint(GLFW_SAMPLES, settings[0]);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, settings[1]);	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, settings[2]);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(settings[3], settings[4], "Planet Generator", NULL, NULL);
	if (window == NULL) {
		std::cout << "Unable to open window. OpenGl version may not to be supported" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cout << "Unable to init GLEW." << std::endl;
		return NULL;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(window, key_callback);

	return window;
}

void Update(GLFWwindow * window) {

	
}

//Обработка нажатий клавиш
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode) {
	//Закрытие окна
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Создаем шейдеры
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Загружаем код шейдеров
	std::string VertexShaderCode = ReadShaderCode(vertex_file_path);
	std::string FragmentShaderCode = ReadShaderCode(fragment_file_path);

	//Компилируем шейдеры
	CompileShader(VertexShaderCode, vertex_file_path, VertexShaderID);
	CompileShader(FragmentShaderCode, fragment_file_path, FragmentShaderID);

	// Создаем шейдерную программу и привязываем шейдеры к ней
	std::cout << "Создаем шейдерную программу и привязываем шейдеры к ней" << std::endl;
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

	return ProgramID;
}

std::string ReadShaderCode(const char * path)
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

int CompileShader(std::string shaderCode, const char * path, GLuint ID)
{
	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Компилируем шейдер
	std::cout << "Компиляция шейдера: " << path;
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
