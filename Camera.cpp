#include "Camera.h"

void Camera::CalcPosition()
{
	position = glm::vec3(
		targetDistance * sinf(inclination) * cosf(azimuth), 
		targetDistance * sinf(inclination) * sinf(azimuth),
		targetDistance * cosf(inclination)
	);
}