#include "Camera.h"
#include <iostream>

glm::mat4 Camera::GetViewMatrix()
{
	if (this->mode == Camera_mode::FREE)
		return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
	else if (this->mode == Camera_mode::ATTACHED)
		return glm::lookAt(this->Position, this->lookAtPoint, this->Up);
}

// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
{
	GLfloat velocity = this->MovementSpeed * deltaTime;
	if (this->mode == Camera_mode::FREE)
	{
		if (direction == FORWARD)
			this->Position += this->Front * velocity;
		if (direction == BACKWARD)
			this->Position -= this->Front * velocity;
		if (direction == LEFT)
			this->Position -= this->Right * velocity;
		if (direction == RIGHT)
			this->Position += this->Right * velocity;
	}
	else if (this->mode == Camera_mode::ATTACHED)
	{
		if (direction == FORWARD)
			this->Position += this->Up * velocity;
		if (direction == BACKWARD)
			this->Position -= this->Up * velocity;
		if (direction == LEFT)
			this->Position += this->Right * velocity;
		if (direction == RIGHT)
			this->Position -= this->Right * velocity;
		this->Position = glm::normalize(this->Position - this->lookAtPoint) * this->Zoom;
		this->updateCameraVectors();
	}
}

// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(GLfloat yoffset, GLfloat xoffset, GLboolean constrainPitch)
{
	if (this->mode == Camera_mode::FREE)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += -xoffset;
		this->Pitch += -yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
	}
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(GLfloat yoffset, GLfloat deltaTime)
{
	if (this->Zoom >= 1.0f +0.1f && this->Zoom <= 45.0f)
		this->Zoom -= this->MovementSpeed *yoffset * deltaTime * this->Zoom / 5.0f;
	if (this->Zoom <= 1.0f + 0.1f)
		this->Zoom = 1.0f + 0.1f;
	if (this->Zoom >= 45.0f)
		this->Zoom = 45.0f;
	this->Position = glm::normalize(this->Position - this->lookAtPoint) * this->Zoom;
}

	// Calculates the front vector from the Camera's (updated) Eular Angles
	void Camera::updateCameraVectors()
	{		
		// Calculate the new Front vector
		glm::vec3 front;
		if (this->mode == Camera_mode::FREE)
		{
			front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
			front.y = sin(glm::radians(this->Pitch));
			front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
		}
		else if (this->mode == Camera_mode::ATTACHED)
			front = this->Position - this->lookAtPoint;

		this->Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		glm::vec3 right_temp = glm::cross(this->Front, this->WorldUp);
		if (glm::length(right_temp) >= 0.05f)
		{
			this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
			this->Up = glm::normalize(glm::cross(this->Right, this->Front));
			this->old_Front = this->Front;
			this->old_Right = this->Right;
			this->old_Up = this->Up;
			this->old_position = this->Position;
		}
		else
		{
			this->Position = this->old_position;
			this->Front = this->old_Front;
			this->Right = old_Right;
			this->Up = this->old_Up;
		}
		
	}