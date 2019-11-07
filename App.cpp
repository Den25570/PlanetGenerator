#include "Window.h"
#include "mesh.h"

using namespace glm;

// 1 - Сглаживание
// 2 - Максимальная версия OGL
// 3 - Минимальная версия OGL
// 4 - Высота окна
// 5 - Ширина окна
int settings[5] = {4,3,3, 1000, 1000};

//Главное окно
GLFWwindow* window;

int main() {
	window = InitWindow(&settings[0]);

	Mesh planetMesh = Mesh::GenerateIsocahedronMesh();
	planetMesh.generateSubdivisions_recursive(1);
	planetMesh.normalizeVertexes(1.0f);

	GLfloat* vertices = (GLfloat*) malloc(planetMesh.vertexes.size() * sizeof(GLfloat)*3);
	GLuint* indices = (GLuint*) malloc(planetMesh.triangles.size() * sizeof(GLuint) * 3);

	for (uint i = 0; i < planetMesh.vertexes.size(); i++)
	{
		vertices[i * 3 + 0] = planetMesh.vertexes[i].x;
		vertices[i * 3 + 1] = planetMesh.vertexes[i].y;
		vertices[i * 3 + 2] = planetMesh.vertexes[i].z;
	}

	int i = 0;
	for (auto it = planetMesh.triangles.cbegin(); it != planetMesh.triangles.cend(); it++, i += 3)
	{
		indices[i + 0] = (*it)[0];
		indices[i + 1] = (*it)[1];
		indices[i + 2] = (*it)[2];
	}
	

	// Создать и откомпилировать нашу шейдерную программу
	GLuint programID = LoadShaders("VertexShader.vert", "FragmentShader.frag");

	//Инициализация буферов
	GLuint EBO, VBO, VAO;
	glGenBuffers(1, &EBO);	
	glGenBuffers(1, &VBO);	
	glGenVertexArrays(1, &VAO);

	//Привязываем
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//STATIC, DYNAMIC, STREAM - в зависимости от частоты изменения данных
	glBufferData(GL_ARRAY_BUFFER, planetMesh.vertexes.size() * sizeof(GLfloat) * 3, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, planetMesh.triangles.size() * sizeof(GLuint) * 3, indices, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//4. Отвязываем VAO
	glBindVertexArray(0);	
	
	//Режим отрисовки
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// Включить тест глубины
	glEnable(GL_DEPTH_TEST);
	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
	glDepthFunc(GL_LESS);


	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//Продолжать работу пока окно не закрыто
	while (!glfwWindowShouldClose(window)) {
		//Проверить ввод
		glfwPollEvents();

		glUseProgram(programID);
		glBindVertexArray(VAO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, planetMesh.triangles.size()*3, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		//Обновить цветовой буфер
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}