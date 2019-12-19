#include "SkyBox.hpp"

SkyBox::SkyBox(Shader* shader) {
	std::vector<float> skyboxVertices = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	vertex_array_object = VAO(shader, std::vector<int> {3}, &skyboxVertices);
	vector<std::string> faces = {
		"cwd_up.JPG",
		"cwd_bk.JPG",
		"cwd_dn.JPG",
		"cwd_bk.JPG",
		"cwd_bk.JPG",
		"cwd_bk.JPG",
	};

	this->cubemapTexture = loadCubemap(faces);
	this->shader = shader;
	this->polygon_mode = GL_TRIANGLES;
	this->draw_mode = GL_FILL;
	this->draw_mode_side = GL_FRONT_AND_BACK;
}

void SkyBox::draw(glm::mat4 * View, glm::mat4 * Projection) {
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	shader->Use();
	vertex_array_object.use(draw_mode_side, draw_mode);
	glm::mat4 cube_view = glm::mat4(glm::mat3(*View));
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, &cube_view[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, &(*Projection)[0][0]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);	
	vertex_array_object.draw(polygon_mode);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}