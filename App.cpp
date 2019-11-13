#include "App.h"

using namespace glm;

// 1 - Сглаживание
// 2 - Максимальная версия OGL
// 3 - Минимальная версия OGL
// 4 - Высота окна
// 5 - Ширина окна
int settings[5] = {4,3,3, 1000, 1000};

//Главное окно
GLFWwindow* window;
Camera* main_camera;
float deltaTime = 1.0f;

//UI
float sphere_radius = 1.0f;
uint sibdivisions_count = 3;

//let _randomLat = [], _randomLon = [];
std::vector<vec3> generateFibonacciSphere(int N,float jitter = 0, float randFloat = 0) {
	std::vector<float> a_latlong;

	// Second algorithm from http://web.archive.org/web/20120421191837/http://www.cgafaq.info/wiki/Evenly_distributed_points_on_sphere
	const float s = 3.6f / sqrtf(N);
	const float dlong = M_PI * (3 - sqrtf(5));  /* ~2.39996323 */
	const float dz = 2.0f / N;
	for (int k = 0, lng = 0, z = 1 - dz / 2; k != N; k++, z -= dz) {
		float r = sqrtf(1 - z * z);
		float latDeg = asin(z) * 180 / M_PI;
		float lonDeg = lng * 180 / M_PI;
		//if (_randomLat[k] == = undefined) _randomLat[k] = randFloat() - randFloat();
		//if (_randomLon[k] == = undefined) _randomLon[k] = randFloat() - randFloat();
		//latDeg += jitter * _randomLat[k] * (latDeg - Math.asin(Math.max(-1, z - dz * 2 * Math.PI * r / s)) * 180 / Math.PI);
		//lonDeg += jitter * _randomLon[k] * (s / r * 180 / Math.PI);
		a_latlong.push_back(latDeg);
		a_latlong.push_back(((int)trunc(lonDeg) % 360) + lonDeg - trunc(lonDeg));
		lng += dlong;
	}
	std::vector<vec3> out;
	for (int r = 0; r < a_latlong.size() / 2; r++) {
		float latRad = a_latlong[r * 2 + 0] / 180.0 * M_PI;
		float lonRad = a_latlong[r * 2 + 1] / 180.0 * M_PI;
		out.push_back(vec3(cosf(latRad) * cosf(lonRad),
			cosf(latRad) * sinf(lonRad),
			sinf(latRad)));
	}
	return out;
}

int main() {
	setlocale(LC_CTYPE, "Russian");

	double lastTime = glfwGetTime();
	std::cout << "Инициализация окна..." << std::endl;

	window = InitWindow(&settings[0]);
	glfwSetKeyCallback(window, key_callback);

	std::cout << "Окно инициализированно. (" << glfwGetTime() - lastTime << "s.)" << std::endl;	
	std::cout << "Генерация точек..." << sibdivisions_count << std::endl;
	lastTime = glfwGetTime();

	//std::vector<vec3> fs = generateFibonacciSphere(N, 0);
	Isocahedron planetMesh = Isocahedron(sibdivisions_count, sphere_radius);

	std::cout << "Генерация точек завершена. Точек сгенерированно: " << planetMesh.verticles.size() << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;	
	std::cout << "Создание триангуляции..." << std::endl;
	lastTime = glfwGetTime();

	TriangleMesh tm = generateDelanuaySphere(&planetMesh.verticles);
	
	std::cout << "Создание триангуляции завершено. Получено треугольников: " << tm.numTriangles << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;

	//Map map = generateMesh(tm, planetMesh.verticles.size(), P, seed);

	GLfloat* vertices;
	GLuint* indices;

	//Подготовка меша к рендеру
	TransformToMesh(vertices, indices, &planetMesh);
	
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
	glBufferData(GL_ARRAY_BUFFER, tm.verticles.size() / 3 * sizeof(GLfloat) * 6, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tm.triangles.size()/3 * sizeof(GLuint) * 6, indices, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	//4. Отвязываем VAO
	glBindVertexArray(0);	

	//Вычисление Камеры
	main_camera = new Camera(4.0f, Camera_mode::ATTACHED);
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	//Режим отрисовки
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// Включить тест глубины
	glEnable(GL_DEPTH_TEST);
	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
	glDepthFunc(GL_LESS);

	//Режим отрисовки
	//glDisable(GL_POLYGON_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	//Продолжать работу пока окно не закрыто	
	while (!glfwWindowShouldClose(window)) {
		double lastTime = glfwGetTime();		

		//Проверить ввод
		glfwPollEvents();

		glUseProgram(programID);
		glBindVertexArray(VAO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		main_camera->CalcPosition();
		glm::mat4 Projection = glm::perspective(glm::radians(main_camera->FOV), (float)width/ (float)height, main_camera->min_p, main_camera->max_p);
		glm::mat4 View = glm::lookAt(main_camera->position, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glDrawElements(GL_TRIANGLES, tm.triangles.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		//Обновить цветовой буфер
		glfwSwapBuffers(window);

	    deltaTime = float(glfwGetTime() - lastTime);
	}

	glfwTerminate();
	return 0;
}

//Подготовка меша к рендеру
void TransformToMesh(GLfloat * &vertices, GLuint * &indices, const TriangleMesh * planetMesh)
{
	vertices = (GLfloat*)malloc(planetMesh->verticles.size() * sizeof(GLfloat) * 6);
	indices = (GLuint*)malloc(planetMesh->triangles.size() * sizeof(GLuint) * 3);

	for (uint i = 0; i < planetMesh->verticles.size(); i++)
	{
		vertices[i * 6 + 0] = planetMesh->verticles[i].x;
		vertices[i * 6 + 1] = planetMesh->verticles[i].y;
		vertices[i * 6 + 2] = planetMesh->verticles[i].z;

		vertices[i * 6 + 3] = (rand() % 101) / 100.0f; //r
		vertices[i * 6 + 4] = (rand() % 101) / 100.0f; //g
		vertices[i * 6 + 5] = (rand() % 101) / 100.0f; //b
	}

	int i = 0;
	for (auto it = planetMesh->triangles.cbegin(); it != planetMesh->triangles.cend(); it++, i += 3)
	{
		indices[i + 0] = (*it)[0];
		indices[i + 1] = (*it)[1];
		indices[i + 2] = (*it)[2];
	}
}

//Обработка нажатий клавиш
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode) {
	//Закрытие окна
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	// Движение вперед
	if (key == GLFW_KEY_W && action == GLFW_REPEAT) {
		main_camera->azimuth += deltaTime * main_camera->movingSpeed;
	}
	// Движение назад
	if (key == GLFW_KEY_S && action == GLFW_REPEAT) {
		main_camera->azimuth -= deltaTime * main_camera->movingSpeed;
	}
	// Стрэйф вправо
	if (key == GLFW_KEY_A && action == GLFW_REPEAT) {
		main_camera->inclination -= deltaTime * main_camera->movingSpeed;
	}
	// Стрэйф влево
	if (key == GLFW_KEY_D && action == GLFW_REPEAT) {
		main_camera->inclination += deltaTime * main_camera->movingSpeed;
	}
}
