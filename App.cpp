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
std::size_t sibdivisions_count = 2;

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

int main() {
	setlocale(LC_CTYPE, "Russian");

	double lastTime = glfwGetTime();
	std::cout << "Инициализация окна..." << std::endl;

	window = InitWindow(&settings[0]);
	glfwSetKeyCallback(window, key_callback);

	std::cout << "Окно инициализированно. (" << glfwGetTime() - lastTime << "s.)" << std::endl;
	std::cout << "Генерация точек..." << sibdivisions_count << std::endl;
	lastTime = glfwGetTime();

	std::vector<vec3> fs = generateFibonacciSphere(N, 0);
    //Isocahedron planetMesh = Isocahedron(sibdivisions_count, sphere_radius);

	std::cout << "Генерация точек завершена. Точек сгенерированно: " << fs.size() << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;
	std::cout << "Создание триангуляции..." << std::endl;
	lastTime = glfwGetTime();

	TriangleMesh tm = generateDelanuaySphere(&fs);

	std::cout << "Создание триангуляции завершено. Получено треугольников: " << tm.numTriangles << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;

	Planet planet = Planet(tm, seed, N, P);
	auto quadGeometry = QuadGeometryV(planet.quadGeometry);

	GLfloat* vertices;
	GLuint* indices;

	//Подготовка меша к рендеру
	drawIndexedTriangles(vertices, indices, &quadGeometry);
	//Инициализация буферов
	GLuint EBO, VBO, VAO;
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	//Привязываем треугольники
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//STATIC, DYNAMIC, STREAM - в зависимости от частоты изменения данных
	glBufferData(GL_ARRAY_BUFFER, quadGeometry.points.size() * sizeof(GLfloat) * 5, vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadGeometry.indices.size() * sizeof(GLuint), indices, GL_STATIC_DRAW);
	// 3. Устанавливаем указатели на вершинные атрибуты 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 *sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	//4. Отвязываем VAO
	glBindVertexArray(0);

	
	GLfloat* vertices_voronoi;
	drawTriangles(vertices_voronoi, &planet.voronoi);
	//Привязывем точки
	GLuint VAO_VORONOI, VBO_VORONOI;
	glGenVertexArrays(1, &VAO_VORONOI);
	glGenBuffers(1, &VBO_VORONOI);

	glBindVertexArray(VAO_VORONOI);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_VORONOI);
	
	glBufferData(GL_ARRAY_BUFFER, planet.voronoi.points.size() * 5 * sizeof(GLfloat), vertices_voronoi, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Привязывем точки
	GLuint VAO_2, VBO_2;
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
    // Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
	glBufferData(GL_ARRAY_BUFFER, planet.map.r_xyz.size() * sizeof(GLfloat), &planet.map.r_xyz[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLfloat* vertices_2;
	//Привязываем линии
	std::vector<vec3> line_xyz = drawPlateBoundaries(vertices_2, &tm, &planet.map);
	GLuint VAO_3, VBO_3;
	glGenVertexArrays(1, &VAO_3);
	glBindVertexArray(VAO_3);
	glGenBuffers(1, &VBO_3);
	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO_3);
	glBufferData(GL_ARRAY_BUFFER, line_xyz.size()*7 * sizeof(GLfloat), vertices_2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLfloat* vertices_3;
	//Привязываем линии
	std::vector<vec3> line_xyz_2 = drawPlateVectors(vertices_3, &tm, &planet.map);
	GLuint VAO_4, VBO_4;
	glGenVertexArrays(1, &VAO_4);
	glBindVertexArray(VAO_4);
	glGenBuffers(1, &VBO_4);
	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO_4);
	glBufferData(GL_ARRAY_BUFFER, line_xyz_2.size() * 7 * sizeof(GLfloat), vertices_3, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	GLfloat* vertices_4;
	//Привязываем линии
	std::vector<vec3> line_xyz_3 = drawRivers(vertices_4, &tm, &planet.map);
	GLuint VAO_5, VBO_5;
	glGenVertexArrays(1, &VAO_5);
	glBindVertexArray(VAO_5);
	glGenBuffers(1, &VBO_5);
	// Allocate space and upload the data from CPU to GPU
	glBindBuffer(GL_ARRAY_BUFFER, VBO_5);
	glBufferData(GL_ARRAY_BUFFER, line_xyz_3.size() * 7 * sizeof(GLfloat), vertices_4, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

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
	Shader indexed_triangle_shader = Shader("VertexShader.vert", "FragmentShader.frag");
	GLuint u_projection = glGetUniformLocation(indexed_triangle_shader.Program, "u_projection");
	GLuint u_colormap = glGetUniformLocation(indexed_triangle_shader.Program, "u_colormap");
	GLuint u_light_angle = glGetUniformLocation(indexed_triangle_shader.Program, "u_light_angle");
	GLuint u_inverse_texture_size = glGetUniformLocation(indexed_triangle_shader.Program, "u_inverse_texture_size");
	GLuint u_d = glGetUniformLocation(indexed_triangle_shader.Program, "u_d");
	GLuint u_c = glGetUniformLocation(indexed_triangle_shader.Program, "u_c");
	GLuint u_slope = glGetUniformLocation(indexed_triangle_shader.Program, "u_slope");
	GLuint u_flat = glGetUniformLocation(indexed_triangle_shader.Program, "u_flat");
	GLuint u_outline_strength = glGetUniformLocation(indexed_triangle_shader.Program, "u_outline_strength");

	Shader triangle_shader = Shader("Triangles.vert", "Triangles.frag");
	GLuint u_projection_voronoi = glGetUniformLocation(triangle_shader.Program, "u_projection");
	GLuint u_colormap_voronoi = glGetUniformLocation(indexed_triangle_shader.Program, "u_colormap");

	Shader pointShader = Shader("Points.vert", "Points.frag");
	GLuint u_projection2 = glGetUniformLocation(pointShader.Program, "u_projection");
	GLuint u_pointsize2 = glGetUniformLocation(pointShader.Program, "u_pointsize");

	Shader lineShader = Shader("Lines.vert", "Lines.frag");
	GLuint u_projection_line = glGetUniformLocation(pointShader.Program, "u_projection");
	GLuint u_multiply_rgba = glGetUniformLocation(pointShader.Program, "u_multiply_rgba");
	GLuint u_add_rgba = glGetUniformLocation(pointShader.Program, "u_add_rgba");

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

		main_camera->CalcPosition();
		glm::mat4 Projection = glm::perspective(glm::radians(main_camera->FOV), (float)width / (float)height, main_camera->min_p, main_camera->max_p);
		glm::mat4 View = glm::lookAt(main_camera->position, glm::vec3(0, 0, 0), glm::vec3(0, 4, 0));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		//Sphere render
		/*indexed_triangle_shader.Use();
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform2f(u_light_angle, cosf(M_PI / 3.0f), sinf(M_PI / 3.0f));
		glUniform1f(u_inverse_texture_size, 1.0f / 4096.0f);
		glUniform1f(u_d, 60.0f);
		glUniform1f(u_c, 0.15f);
		glUniform1f(u_slope, 6.0f);
		glUniform1f(u_flat, 2.5f);
		glUniform1f(u_outline_strength, 5.0f);
		glUniformMatrix4fv(u_projection, 1, GL_FALSE, &MVP[0][0]);
		glDrawElements(GL_TRIANGLES, quadGeometry.indices.size(), GL_UNSIGNED_INT, 0);*/

		triangle_shader.Use();
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_VORONOI);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniformMatrix4fv(u_projection_voronoi, 1, GL_FALSE, &MVP[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, planet.voronoi.points.size() * 5);
		glBindVertexArray(0);

		//line render
		/*lineShader.Use();
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_3);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0,0,0,0);
		glUniform4f(u_multiply_rgba, 1,1,1,1);
		glDrawArrays(GL_LINES, 0, line_xyz.size()*7);*/

		//line render
		/*lineShader.Use();
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_4);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0, 0, 0, 0);
		glUniform4f(u_multiply_rgba, 1, 1, 1, 1);
		glDrawArrays(GL_LINES, 0, line_xyz_2.size() * 7);*/

		//line render
		lineShader.Use();
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_5);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0, 0, 0, 0);
		glUniform4f(u_multiply_rgba, 1, 1, 1, 1);
		glDrawArrays(GL_LINES, 0, line_xyz_3.size() * 7);

		//Point render
		/*pointShader.Use();
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_2);
		glUniformMatrix4fv(u_projection2, 1, GL_FALSE, &MVP[0][0]);
		glUniform1f(u_pointsize2, 0.1f + 100.0f / sqrt(map.r_xyz.size() / 3));
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
		glDrawArrays(GL_POINTS, 0, map.r_xyz.size()/3);*/

		//Обновить цветовой буфер
		glfwSwapBuffers(window);

		deltaTime = float(glfwGetTime() - lastTime);
	}

	glfwTerminate();
	return 0;
}

//Подготовка меша к рендеру
void drawIndexedTriangles(GLfloat * &vertices, GLuint * &indices, const QuadGeometryV * planetMesh)
{
	vertices = (GLfloat*)malloc(planetMesh->points.size() * sizeof(GLfloat) * 5);
	indices = (GLuint*)malloc(planetMesh->indices.size() * sizeof(GLuint));

	std::vector<GLfloat> v;

	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		vertices[i * 5 + 0] = planetMesh->points[i].x;
		vertices[i * 5 + 1] = planetMesh->points[i].y;
		vertices[i * 5 + 2] = planetMesh->points[i].z;

		vertices[i * 5 + 3] = planetMesh->tem_mois[i].x;
		vertices[i * 5 + 4] = planetMesh->tem_mois[i].y;
	}
	for (int i = 0; i < planetMesh->indices.size(); i++)
		indices[i] = planetMesh->indices[i];
}

void drawTriangles(GLfloat * &vertices, const Voronoi * planetMesh)
{
	vertices = (GLfloat*)malloc(planetMesh->points.size() * sizeof(GLfloat) * 5);

	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		vertices[i * 5 + 0] = planetMesh->points[i].x;
		vertices[i * 5 + 1] = planetMesh->points[i].y;
		vertices[i * 5 + 2] = planetMesh->points[i].z;

		vertices[i * 5 + 3] = planetMesh->tm[i].x;
		vertices[i * 5 + 4] = planetMesh->tm[i].y;
	}
}

std::vector<vec3> drawPlateBoundaries(GLfloat * &vertices, TriangleMesh * mesh, Map * map) {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int s = 0; s < mesh->numSides; s++) {
		int begin_r = mesh->s_begin_r(s);
		int	end_r = mesh->s_end_r(s);
		if (map->plates.r_plate[begin_r] != map->plates.r_plate[end_r]) {
			int inner_t = mesh->s_inner_t(s);
			int	outer_t = mesh->s_outer_t(s);

			line_xyz.push_back(vec3(map->t_xyz[3 * inner_t + 0], map->t_xyz[3 * inner_t + 1], map->t_xyz[3 * inner_t + 2]));
			line_xyz.push_back(vec3(map->t_xyz[3 * outer_t + 0], map->t_xyz[3 * outer_t + 1], map->t_xyz[3 * outer_t + 2]));

			line_rgba.push_back(vec4(1, 1, 1, 1));
			line_rgba.push_back(vec4(1, 1, 1, 1));
		}
	}

	vertices = (GLfloat*)malloc(line_xyz.size() * sizeof(GLfloat) * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		vertices[i * 7 + 0] = line_xyz[i].x;
		vertices[i * 7 + 1] = line_xyz[i].y;
		vertices[i * 7 + 2] = line_xyz[i].z;
		vertices[i * 7 + 3] = line_rgba[i].r;
		vertices[i * 7 + 4] = line_rgba[i].g;
		vertices[i * 7 + 5] = line_rgba[i].b;
		vertices[i * 7 + 6] = line_rgba[i].a;
	}
	return line_xyz;
}

std::vector<vec3> drawPlateVectors(GLfloat * &vertices, TriangleMesh * mesh, Map * map) {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int r = 0; r < mesh->numRegions; r++) {
		line_xyz.push_back(vec3(map->r_xyz[r*3+0], map->r_xyz[r * 3 + 1], map->r_xyz[r * 3 + 2]));
		line_rgba.push_back(vec4(1, 1, 1, 1));
		line_xyz.push_back(vec3(map->r_xyz[r * 3 + 0], map->r_xyz[r * 3 + 1], map->r_xyz[r * 3 + 2]) + map->plates.plate_vec[map->plates.r_plate[r]] * (2 / sqrtf(1000)));
		line_rgba.push_back(vec4(1, 0, 0, 0));
	}

	vertices = (GLfloat*)malloc(line_xyz.size() * sizeof(GLfloat) * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		vertices[i * 7 + 0] = line_xyz[i].x;
		vertices[i * 7 + 1] = line_xyz[i].y;
		vertices[i * 7 + 2] = line_xyz[i].z;
		vertices[i * 7 + 3] = line_rgba[i].r;
		vertices[i * 7 + 4] = line_rgba[i].g;
		vertices[i * 7 + 5] = line_rgba[i].b;
		vertices[i * 7 + 6] = line_rgba[i].a;
	}
	return line_xyz;
}

std::vector<vec3> drawRivers(GLfloat * &vertices, TriangleMesh * mesh, Map * map) {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int s = 0; s < mesh->numSides; s++) {
		if (map->s_flow[s] > 1) {
			float flow = 0.1f * sqrtf(map->s_flow[s]);
			int inner_t = mesh->s_inner_t(s);
			int	outer_t = mesh->s_outer_t(s);
			line_xyz.push_back(vec3(map->t_xyz[3 * inner_t], map->t_xyz[3 * inner_t + 1], map->t_xyz[3 * inner_t + 2]));
			line_xyz.push_back(vec3(map->t_xyz[3 * outer_t], map->t_xyz[3 * outer_t + 1], map->t_xyz[3 * outer_t + 2]));
			if (flow > 1) flow = 1;
			vec4 rgba_premultiplied = vec4(0.2f * flow, 0.5f * flow, 0.7f * flow, flow);
			line_rgba.push_back(rgba_premultiplied);
			line_rgba.push_back(rgba_premultiplied);
		}
	}
	vertices = (GLfloat*)malloc(line_xyz.size() * sizeof(GLfloat) * 7);
	for (int i = 0; i < line_xyz.size(); i++) {
		vertices[i * 7 + 0] = line_xyz[i].x;
		vertices[i * 7 + 1] = line_xyz[i].y;
		vertices[i * 7 + 2] = line_xyz[i].z;
		vertices[i * 7 + 3] = line_rgba[i].r;
		vertices[i * 7 + 4] = line_rgba[i].g;
		vertices[i * 7 + 5] = line_rgba[i].b;
		vertices[i * 7 + 6] = line_rgba[i].a;
	}
	return line_xyz;
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


