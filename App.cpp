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
uint sibdivisions_count = 2;

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

	return window;
}

std::vector<vec3> generateFibonacciSphere(int N, float jitter = 0, float randFloat = 0) {
	std::vector<float> a_latlong;

	// Second algorithm from http://web.archive.org/web/20120421191837/http://www.cgafaq.info/wiki/Evenly_distributed_points_on_sphere
	const float s = 3.6f / sqrtf(N);
	const float dlong = M_PI * (3 - sqrtf(5));  /* ~2.39996323 */
	const float dz = 2.0f / N;
	for (float k = 0, lng = 0, z = 1 - dz / 2; k != N; k++, z -= dz) {
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

	std::vector<vec3> fs = generateFibonacciSphere(1000, 0);
    //Isocahedron planetMesh = Isocahedron(sibdivisions_count, sphere_radius);

	std::cout << "Генерация точек завершена. Точек сгенерированно: " << fs.size() << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;
	std::cout << "Создание триангуляции..." << std::endl;
	lastTime = glfwGetTime();

	TriangleMesh tm = generateDelanuaySphere(&fs);

	std::cout << "Создание триангуляции завершено. Получено треугольников: " << tm.numTriangles << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;

	QuadGeometry quadg;
	Map map = generateMesh(tm, tm.points.size(), P, 999, &quadg);
	QuadGeometryV qg = QuadGeometryV(quadg);

	GLfloat* vertices;
	GLuint* indices;

	//Подготовка меша к рендеру
	TransformToMesh(vertices, indices, &qg);


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
	glBufferData(GL_ARRAY_BUFFER, qg.points.size() * sizeof(GLfloat) * 5, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, qg.indices.size() * sizeof(GLuint), indices, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	//4. Отвязываем VAO
	glBindVertexArray(0);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, map.r_xyz.size() * sizeof(GLfloat), &map.r_xyz[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//Вычисление Камеры
	main_camera = new Camera(4.0f, Camera_mode::ATTACHED);

	// Включить тест глубины
	glEnable(GL_DEPTH_TEST);
	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
	glDepthFunc(GL_LESS);

	//Режим отрисовки
	//glDisable(GL_POLYGON_SMOOTH);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	//Загрузка и настройки шейдеров
	Shader triangleShader = Shader("VertexShader.vert", "FragmentShader.frag");
	GLuint u_projection = glGetUniformLocation(triangleShader.Program, "u_projection");
	GLuint u_colormap = glGetUniformLocation(triangleShader.Program, "u_colormap");
	GLuint u_light_angle = glGetUniformLocation(triangleShader.Program, "u_light_angle");
	GLuint u_inverse_texture_size = glGetUniformLocation(triangleShader.Program, "u_inverse_texture_size");
	GLuint u_d = glGetUniformLocation(triangleShader.Program, "u_d");
	GLuint u_c = glGetUniformLocation(triangleShader.Program, "u_c");
	GLuint u_slope = glGetUniformLocation(triangleShader.Program, "u_slope");
	GLuint u_flat = glGetUniformLocation(triangleShader.Program, "u_flat");
	GLuint u_outline_strength = glGetUniformLocation(triangleShader.Program, "u_outline_strength");

	Shader pointShader = Shader("Points.vert", "Points.frag");
	GLuint u_projection2 = glGetUniformLocation(pointShader.Program, "u_projection");
	GLuint u_pointsize2 = glGetUniformLocation(pointShader.Program, "u_pointsize");

	//Настройка текстуры
	int t_width = 64, t_height = 64;
	std::vector<unsigned char> colorm = colormap(t_width, t_height);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t_width, t_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colorm[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);


	//Продолжать работу пока окно не закрыто	
	while (!glfwWindowShouldClose(window)) {
		double lastTime = glfwGetTime();

		//Проверить ввод
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		triangleShader.Use();
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(VAO);		

		main_camera->CalcPosition();
		glm::mat4 Projection = glm::perspective(glm::radians(main_camera->FOV), (float)width / (float)height, main_camera->min_p, main_camera->max_p);
		glm::mat4 View = glm::lookAt(main_camera->position, glm::vec3(0, 0, 0), glm::vec3(0, 4, 0));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform2f(u_light_angle, cosf(M_PI / 3.0f), sinf(M_PI / 3.0f));
		glUniform1f(u_inverse_texture_size, 1.0f / 4098.0f);
		glUniform1f(u_d, 60.0f);
		glUniform1f(u_c, 0.15f);
		glUniform1f(u_slope, 6.0f);
		glUniform1f(u_flat, 2.5f);
		glUniform1f(u_outline_strength, 5.0f);
		glUniformMatrix4fv(u_projection, 1, GL_FALSE, &MVP[0][0]);

		glDrawElements(GL_TRIANGLES, qg.indices.size(), GL_UNSIGNED_INT, 0);

		pointShader.Use();
		glBindVertexArray(vao);
		glUniformMatrix4fv(u_projection2, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(u_pointsize2, 0.1f + 100.0f / sqrt(map.r_xyz.size() / 3));
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
		glDrawArrays(GL_POINTS, 0, map.r_xyz.size()/3);

		//Обновить цветовой буфер
		glfwSwapBuffers(window);

		deltaTime = float(glfwGetTime() - lastTime);
	}

	glfwTerminate();
	return 0;
}

//Подготовка меша к рендеру
void TransformToMesh(GLfloat * &vertices, GLuint * &indices, const QuadGeometryV * planetMesh)
{
	vertices = (GLfloat*)malloc(planetMesh->points.size() * sizeof(GLfloat) * 5);
	indices = (GLuint*)malloc(planetMesh->indices.size() * sizeof(GLuint));

	for (uint i = 0; i < planetMesh->points.size(); i++)
	{
		vertices[i * 5 + 0] = planetMesh->points[i].x;
		vertices[i * 5 + 1] = planetMesh->points[i].y;
		vertices[i * 5 + 2] = planetMesh->points[i].z;

		vertices[i * 5 + 3] = planetMesh->tem_mois[i].x;
		vertices[i * 5 + 4] = planetMesh->tem_mois[i].y;
	}

	/*int i = 0;
	for (auto it = planetMesh->triangles.cbegin(); it != planetMesh->triangles.cend(); it++, i += 3)
	{
		indices[i + 0] = (*it)[0];
		indices[i + 1] = (*it)[1];
		indices[i + 2] = (*it)[2];
	}*/
	for (int i = 0; i < planetMesh->indices.size(); i++)
		indices[i] = planetMesh->indices[i];
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
