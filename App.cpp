#include "App.h"
#include <string>

using namespace glm;

// 1 - Multisampling
// 2 - Gl version max
// 3 - Gl version min
// 4 - Window init width 
// 5 - Window init height
int settings[5] = {4,3,3, 1600, 900};

GLFWwindow* window;
Camera camera(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), 4.0f);

//Mouse & keyboard input
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

//Render settings
bool bloom = true;
bool bloomKeyPressed = false;
bool drawNoiseFlag = false;
bool drawRiversFlag = false;
bool illuminate_entire_surface = false;
float exposure = 1.0f;
float outline_strength = 1.0f;
float ocean_percent = 70.0f;
float temperature_change = 0.0f;

float zoom = 4.0f;
float angle = 0;

//Generate settings
int region_number = 250;
int plates_number = 20;
int seed = 12300;

//utils
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float FOV = 90.0f;
bool start_generate_planet = false;

std::vector<Planet*> planets = std::vector<Planet*>(10, new Planet());

int main() {
	

	window = InitWindow(&settings[0]);	
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	//Imgui library int
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//Compiling Shaders 
	Shader skyboxShader = Shader("Shaders/skybox.vert", "Shaders/skybox.frag");
	Shader indexed_triangle_shader = Shader("Shaders/VertexShader.vert", "Shaders/FragmentShader.frag");
	Shader triangle_shader = Shader("Shaders/Triangles.vert", "Shaders/Triangles.frag");
	Shader pointShader = Shader("Shaders/Points.vert", "Shaders/Points.frag");
	Shader lineShader = Shader("Shaders/Lines.vert", "Shaders/Lines.frag");
	Shader bloomShader = Shader("Shaders/FrameQuad.vert", "Shaders/Gauss.frag");
	Shader blurFrameShader = Shader("Shaders/FrameQuad.vert", "Shaders/FrameQuad.frag");

	//Setting layout for textures
	blurFrameShader.Use();
	blurFrameShader.setInt("image", 0);
	bloomShader.Use();
	bloomShader.setInt("scene", 0);
	bloomShader.setInt("bloomBlur", 1);

	GLuint planet_texture = generatePlanetTexture();
	SkyBox sky_box = SkyBox(&skyboxShader);

	std::vector<float> quadVerticles = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	VAO frame_quad_VAO = VAO(&blurFrameShader, std::vector<int> {2,2}, &quadVerticles);

	//Frame buffer init
	unsigned int frame_buffer_object;
	glGenFramebuffers(1, &frame_buffer_object);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
	// Create 2 floating point color buffers (1 for regular rendering, other for brightness treshold values)
	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, display_w, display_h, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// Create and attach depth buffer
	unsigned int render_buffer_object_depth;
	glGenRenderbuffers(1, &render_buffer_object_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, display_w, display_h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object_depth);
	// Create and attach color attachments 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping-pong-framebuffer for blurring
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, display_w, display_h, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	//Weight coefficients for Euler's curve
	std::vector<float> weight = {0.0, 0.0, 0.0, 0.072, 0.264};
	int pass_amount = 0;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;
	style.Alpha = 0.8f;

	glEnable(GL_MULTISAMPLE);
	//Main loop
	while (!glfwWindowShouldClose(window)) {
		double lastTime = glfwGetTime();

		glfwPollEvents();
		UpdateCamera();

		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		//gui render init
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//Main gui interface
		{
			static int current_planet = 0;
			static vec3 position = vec3(0, 0, 0);
			ImGui::SetNextWindowPos({ 0, (float)display_h }, 0, { 0,1 });
			ImGui::SetNextWindowSize({ 0,0 }, 0);
			ImGui::Begin(" ");

			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("FOV", &FOV, 30.0f, 180.0f);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("wiew distance", &camera.Zoom, 1.0f, 40.0f);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputInt("Seed", &seed);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputInt("Regions number", &region_number, 100);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputInt("Plates Number", &plates_number);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("Water Percent", &ocean_percent, 0.0f, 100.0f);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("Temperature change", &temperature_change, -10.0f, 10.0f);

			//ImGui::SetNextItemWidth(display_w / 10.f);
			//ImGui::SliderInt("Current Planet index", &current_planet, 0, 9);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("Rotating speed", &(*planets[current_planet]).rotating_speed, 0.0f, 10.0f);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputFloat3("Axis", &(*planets[current_planet]).axis.x);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputFloat3("Position", &position.x);

			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::InputInt("Pass amount", &pass_amount);
			pass_amount = pass_amount < 0 ? 0 : pass_amount;
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("outline strength", &outline_strength, 0.0f, 10.0f);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("w4", &weight[3], 0, 0.5);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::SliderFloat("w5", &weight[4], 0, 0.5);

			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::Checkbox("Noise Render", &drawNoiseFlag);
			ImGui::SetNextItemWidth(display_w / 10.f);
			ImGui::Checkbox("Rivers Render", &drawRiversFlag);

			ImGui::SetNextItemWidth(display_w / 10.f);
			if (ImGui::Button("Generate Planet")) {
				planets[0] = new Planet(region_number, plates_number, seed, ocean_percent);
				planets[0]->position = position;
				planets[0]->setVAOs(&indexed_triangle_shader, &triangle_shader, &pointShader, &lineShader);
			}

			ImGui::End();
		}
		//Frame per second display
		{
			bool isOpen = true;
			ImGuiWindowFlags window_flags = 0;
			window_flags |= ImGuiWindowFlags_NoTitleBar | 
							ImGuiWindowFlags_NoBackground;

			ImGui::SetNextWindowPos({ 0, 0 }, 0);
			ImGui::SetNextWindowSize({ 0,0 }, 0);
			
			ImGui::Begin("Framerate", &isOpen, window_flags);
			ImGui::BulletText(&std::to_string(1 / deltaTime)[0]);
			ImGui::End();
			
		}

		ImGui::Render();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//main render init
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glActiveTexture(GL_TEXTURE0);
		/////////////////////////////////////////////////////////////////////
		 
		glm::mat4 Projection = glm::perspective( FOV/180.0f* (float)M_PI, (float)display_w / (float)display_h, 0.1f, 1000.0f);
		glm::mat4 View = camera.GetViewMatrix();	

		//Planet render
		for (int i = 0 ; i < planets.size(); i++)
		if (planets[i]->initialized) {

			glm::mat4 Model = glm::mat4(1.0f);
			angle += (planets[i]->rotating_speed * deltaTime);
			Model = glm::rotate(Model, angle, planets[i]->axis);
			glm::mat4 MVP = Projection * View * Model;

			planets[i]->texture = planet_texture;
			planets[i]->draw(drawNoiseFlag, drawRiversFlag, &MVP, &Model , outline_strength);
		}
		
		//Ping-Pong blur iterations
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		bool horizontal = true, first_iteration = true;
		int amount = pass_amount;
		bloomShader.Use();
		for (int i = 0; i < amount; i++)
		{		
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			
			glUniform1fv(glGetUniformLocation(bloomShader.Program, "weight"),5, &weight[0]);
			glUniform1i(glGetUniformLocation(bloomShader.Program, "horizontal"), horizontal);
			glBindTexture(
				GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]
			);
			frame_quad_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
			frame_quad_VAO.draw(GL_TRIANGLES);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		
		//SkyBox render
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
		sky_box.draw(&View, &Projection);

		drawFrameBuffer(frame_quad_VAO, blurFrameShader, colorBuffers, pingpongColorbuffers, horizontal);

		//Final gui draw
		glDisable(GL_DEPTH_TEST);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//Update color buffers
		glfwSwapBuffers(window);

		deltaTime = float(glfwGetTime() - lastTime);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void drawFrameBuffer(VAO &frame_quad_VAO, Shader &blurFrameShader, unsigned int  colorBuffers[2], unsigned int  pingpongColorbuffers[2], bool horizontal)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	blurFrameShader.Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
	blurFrameShader.setInt("bloomBlur", bloom);
	blurFrameShader.setFloat("exposure", exposure);
	frame_quad_VAO.use(GL_FRONT_AND_BACK, GL_FILL);
	frame_quad_VAO.draw(GL_TRIANGLES);
}

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
	glfwSwapInterval(1); // Enable vsync

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cout << "Unable to init GLEW." << std::endl;
		return NULL;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return window;
}

void UpdateCamera()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Called on input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//cout << key << endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


