#ifndef CAMERA_H
#define CAMERA_H

#include "glew.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "CollisionManager.h"
#include <iostream>
#include <vector>

// Camera movement options abstracted as enums for ease of use and separation from system controls.
enum Movement {
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT
};

// Default camera values. Playing with the speed and sensitivity is okay, but for anything else, you'll be playing a dangerous game, muhahaha!
const float YAW        = -90.0f;
const float PITCH      = 0.0f;
const float SPEED      = 4.5f;
const float SENSITIVTY = 0.15f;
const float ZOOM       = 45.0f;

// Camera class that allows for easy modification of position, target, and zoom in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front = glm::vec3(0, 0, -1);
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 PlayerElipse;
	// Eular Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed    = SPEED;
	float MouseSensitivity = SENSITIVTY;
	float Zoom             = ZOOM;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 up = glm::vec3(0, 1, 0), float yaw = YAW, float pitch = PITCH)
	{
		this->Position = position;
		this->WorldUp  = up;
		this->Yaw      = yaw;
		this->Pitch    = pitch;
		this->PlayerElipse = glm::vec3(1, 1, 1);
		updateCameraVectors();
	}

	// Returns the view matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	glm::vec3 getPosition()
	{
		return Position;
	}

	// Processes movement. Uses Movement enum to keep independant from input type (keyboard, controller, mouse, etc).
	void ProcessMovement(Movement direction, float steps)
	{
		float speed = MovementSpeed * steps;
		glm::vec3 velocity;

		switch (direction) {
		case MOVE_FORWARD:
			velocity = speed * glm::vec3(Front.x, 0.0f, Front.z);
			break;
		case MOVE_BACKWARD:
			velocity = -speed * glm::vec3(Front.x, 0.0f, Front.z);
			break;
		case MOVE_LEFT:
			velocity = -speed * glm::vec3(Right.x, 0.0f, Right.z);
			break;
		case MOVE_RIGHT:
			velocity = speed * glm::vec3(Right.x, 0.0f, Right.z);
			break;
		}
		CollisionManager::getInstance()->askMove(PlayerElipse, velocity, Position);
		Position += velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset)
	{
		Yaw   += xoffset * MouseSensitivity;
		Pitch += yoffset * MouseSensitivity;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 Front;
		Front.x     = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front.y     = sin(glm::radians(Pitch));
		Front.z     = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		// Normalize the vectors because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->Front = glm::normalize(Front);
		// Also re-calculate the Right and Up vector
		this->Right = glm::normalize(glm::cross(Front, WorldUp));
		this->Up    = glm::normalize(glm::cross(Right, Front));
	}
};
#endif
