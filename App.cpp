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

//Вынести в UI
float zoom = 4.0f;
int region_number = 50000;
int plates_number = 20;
int seed = 12300;

//Освещение


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

	window = InitWindow(&settings[0]);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	std::vector<vec3> fs = generateFibonacciSphere(region_number, 0.1f);
	TriangleMesh * tm = generateDelanuaySphere(&fs);
	Planet planet = Planet(*tm, seed, region_number, plates_number);

	fs.clear();
	fs.shrink_to_fit();

	std::cout << "Генерация завершена." << " (" << glfwGetTime() - lastTime << "s.)" << std::endl;

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
	GLuint model_voronoi = glGetUniformLocation(triangle_shader.Program, "model");
	GLuint light_color = glGetUniformLocation(triangle_shader.Program, "light_color");
	GLuint ambitient_strength = glGetUniformLocation(triangle_shader.Program, "ambitient_strength");
	GLuint light_pos = glGetUniformLocation(triangle_shader.Program, "light_pos");
	GLuint u_colormap_voronoi = glGetUniformLocation(indexed_triangle_shader.Program, "u_colormap");

	Shader pointShader = Shader("Points.vert", "Points.frag");
	GLuint u_projection2 = glGetUniformLocation(pointShader.Program, "u_projection");
	GLuint u_pointsize2 = glGetUniformLocation(pointShader.Program, "u_pointsize");

	Shader lineShader = Shader("Lines.vert", "Lines.frag");
	GLuint u_projection_line = glGetUniformLocation(pointShader.Program, "u_projection");
	GLuint u_multiply_rgba = glGetUniformLocation(pointShader.Program, "u_multiply_rgba");
	GLuint u_add_rgba = glGetUniformLocation(pointShader.Program, "u_add_rgba");

	//
	std::vector<GLfloat> vertices; std::vector<GLuint> indices ;
	drawIndexedTriangles(vertices, indices, &planet.quadGeometryV);
	VAO noise_VAO = VAO(&indexed_triangle_shader, std::vector<int> {3,2}, &vertices, &indices);
	//
	std::vector<GLfloat> vertices_voronoi;
	drawTriangles(vertices_voronoi, &planet.voronoi);
	VAO voronoi_VAO = VAO(&triangle_shader, std::vector<int> {3, 2}, &vertices_voronoi);	
	//
	VAO points_VAO = VAO(&pointShader, std::vector<int> {3}, &planet.map.r_xyz);
	//
	std::vector<GLfloat> vertices_2;
	drawPlateBoundaries(vertices_2, tm, &planet.map);
	VAO plate_boundaries_VAO = VAO(&lineShader, std::vector<int> {3, 4}, &vertices_2);
	//
	std::vector<GLfloat> vertices_3;
	drawPlateVectors(vertices_3, tm, &planet.map);
	VAO plate_vectors_VAO = VAO(&lineShader, std::vector<int> {3, 4}, &vertices_3);
	//
	std::vector<GLfloat> vertices_4;
	drawRivers(vertices_4, tm, &planet.map);
	VAO rivers_VAO = VAO(&lineShader, std::vector<int> {3, 4}, &vertices_4);

	//Освобождение ресурсов
	vertices.clear();
	indices.clear();
	vertices_voronoi.clear();
	vertices_3.clear();
	vertices_4.clear();

	//Вычисление Камеры
	main_camera = new Camera(4.0f, Camera_mode::ATTACHED);

	// Включить тест глубины
	glEnable(GL_DEPTH_TEST);
	// Фрагмент будет выводиться только в том, случае, если он находится ближе к камере, чем предыдущий
	glDepthFunc(GL_LESS);

	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	//Настройка текстуры
	int t_width = 64, t_height = 64;
	std::vector<unsigned char> colorm = colormap(t_width, t_height);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t_width, t_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colorm[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_DEPTH_TEST);

	//Продолжать работу пока окно не закрыто	
	while (!glfwWindowShouldClose(window)) {
		double lastTime = glfwGetTime();

		//Проверить ввод
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		main_camera->targetDistance = zoom;
		main_camera->CalcPosition();
		glm::mat4 Projection = glm::perspective(glm::radians(main_camera->FOV), (float)width / (float)height, main_camera->min_p, main_camera->max_p);
		glm::mat4 View = glm::lookAt(main_camera->position, glm::vec3(0, 0, 0), glm::vec3(0, 4, 0));
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * View * Model;

		//Sphere render
		/*noise_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform2f(u_light_angle, cosf(M_PI / 3.0f), sinf(M_PI / 3.0f));
		glUniform1f(u_inverse_texture_size, 1.0f / 2048.0f);
		glUniform1f(u_d, 60.0f);
		glUniform1f(u_c, 0.15f);
		glUniform1f(u_slope, 6.0f);
		glUniform1f(u_flat, 2.5f);
		glUniform1f(u_outline_strength, 5.0f);
		glUniformMatrix4fv(u_projection, 1, GL_FALSE, &MVP[0][0]);
		noise_VAO.draw();*/
		
		voronoi_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform3f(light_color, 1, 1, 1);
		glUniform3f(light_pos, 20, 0, 0);
		glUniform1f(ambitient_strength, 0.15f);
		glUniformMatrix4fv(model_voronoi, 1, GL_FALSE, &Model[0][0]);
		glUniformMatrix4fv(u_projection_voronoi, 1, GL_FALSE, &MVP[0][0]);
		voronoi_VAO.draw(GL_TRIANGLES);

		/*plate_vectors_VAO.use(GL_FRONT_AND_BACK, GL_LINE);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0,0,0,0);
		glUniform4f(u_multiply_rgba, 1,1,1,1);
		plate_vectors_VAO.draw(GL_LINES);

		plate_boundaries_VAO.use(GL_FRONT_AND_BACK, GL_LINE);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0, 0, 0, 0);
		glUniform4f(u_multiply_rgba, 1, 1, 1, 1);
		plate_boundaries_VAO.draw(GL_LINES);*/

		//line render
		/*lineShader.Use();
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(VAO_5);
		glUniformMatrix4fv(u_projection_line, 1, GL_FALSE, &MVP[0][0]);
		glUniform4f(u_add_rgba, 0, 0, 0, 0);
		glUniform4f(u_multiply_rgba, 1, 1, 1, 1);
		glDrawArrays(GL_LINES, 0, line_xyz_3.size() * 7);*/

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
void drawTriangleMesh(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const TriangleMesh * planetMesh)
{
	vertices = std::vector<GLfloat>(planetMesh->points.size() * 5);
	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		(vertices)[i * 5 + 0] = planetMesh->points[i].x;
		(vertices)[i * 5 + 1] = planetMesh->points[i].y;
		(vertices)[i * 5 + 2] = planetMesh->points[i].z;
		(vertices)[i * 5 + 3] = 0;
		(vertices)[i * 5 + 4] = 0;
	}
	indices = std::vector<GLuint>(planetMesh->triangles.size());
	for (int i = 0; i < planetMesh->triangles.size(); i++)
		(indices)[i] = planetMesh->triangles[i];
}

//Подготовка меша к рендеру
void drawIndexedTriangles(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, const QuadGeometryV * planetMesh)
{
	vertices =  std::vector<GLfloat>(planetMesh->points.size() * 5);
	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		(vertices)[i * 5 + 0] = planetMesh->points[i].x;
		(vertices)[i * 5 + 1] = planetMesh->points[i].y;
		(vertices)[i * 5 + 2] = planetMesh->points[i].z;
		(vertices)[i * 5 + 3] = planetMesh->tem_mois[i].x;
		(vertices)[i * 5 + 4] = planetMesh->tem_mois[i].y;
	}
	indices =  std::vector<GLuint>(planetMesh->indices.size());
	for (int i = 0; i < planetMesh->indices.size(); i++)
		(indices)[i] = planetMesh->indices[i];
}

void drawTriangles(std::vector<GLfloat> &vertices, const Voronoi * planetMesh)
{
	vertices =  std::vector<GLfloat>(planetMesh->points.size() * 5);
	for (std::size_t i = 0; i < planetMesh->points.size(); i++)
	{
		vertices[i * 5 + 0] = planetMesh->points[i].x;
		vertices[i * 5 + 1] = planetMesh->points[i].y;
		vertices[i * 5 + 2] = planetMesh->points[i].z;
		vertices[i * 5 + 3] = planetMesh->tm[i].x;
		vertices[i * 5 + 4] = planetMesh->tm[i].y;		
	}
}

std::vector<vec3> drawPlateBoundaries(std::vector<GLfloat> & vertices, TriangleMesh * mesh, Map * map) {
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

	vertices =  std::vector<GLfloat>(line_xyz.size() * 7);
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

std::vector<vec3> drawPlateVectors(std::vector<GLfloat> & vertices, TriangleMesh * mesh, Map * map) {
	std::vector<vec3> line_xyz;
	std::vector<vec4> line_rgba;
	for (int r = 0; r < mesh->numRegions; r++) {
		line_xyz.push_back(vec3(map->r_xyz[r*3+0], map->r_xyz[r * 3 + 1], map->r_xyz[r * 3 + 2]));
		line_rgba.push_back(vec4(1, 1, 1, 1));
		line_xyz.push_back(vec3(map->r_xyz[r * 3 + 0], map->r_xyz[r * 3 + 1], map->r_xyz[r * 3 + 2]) + map->plates.plate_vec[map->plates.r_plate[r]] * (2 / sqrtf(1000)));
		line_rgba.push_back(vec4(1, 0, 0, 0));
	}

	vertices =  std::vector<GLfloat>(line_xyz.size() * 7);
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

std::vector<vec3> drawRivers(std::vector<GLfloat> & vertices, TriangleMesh * mesh, Map * map) {
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

	vertices =  std::vector<GLfloat>(line_xyz.size() * 7);
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom += yoffset / 10;
}


