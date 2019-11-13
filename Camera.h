#pragma once

#define GLEW_STATIC
#include <gl/glew.h>
#include  <gl/glfw3.h>

#include  <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>

enum Camera_mode {FREE, ATTACHED};

class Camera {
public:
	glm::vec3 position;
	Camera_mode mode;
	GLfloat FOV = 45.0f;

	GLfloat min_p = 0.1f;
	GLfloat max_p = 100.0f;

	GLfloat targetDistance;
	GLfloat inclination = 0;
	GLfloat azimuth = 0;

	GLfloat movingSpeed = 10.0f;

	Camera(GLfloat _targetDistance, Camera_mode _mode) {
		targetDistance = _targetDistance;
		mode = _mode;
	};
	~Camera() {};

	void CalcPosition();
private:

};
