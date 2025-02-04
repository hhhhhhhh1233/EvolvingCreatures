#pragma once
#include "core/math/mat4.h"

// For easier camera handling and encapsulation
class Camera
{
public:
	/// BASIC CAMERA PROPERTIES
	vec3 position;
	vec3 target;
	vec3 up;
	float yaw, pitch;

	/// CONTROLLABLE CAMERA MOVEMENT PROPERTIES
	const float verticalLimit = 3.14/2; /// How many radians up or down the camera can move
	float speed = 3;
	float mouseSpeed = 0.5;
	double horizontalAngle = 3.141592;
	double verticalAngle = 0;
	bool bFirstPress = true;
	double xOldPos, yOldPos;

	Camera()
	{
		position = vec3(0, 0, 0);
		target = vec3(0, 0, 0);
		up = vec3(0, 1, 0);
	}

	Camera(vec3 CameraPosition, vec3 CameraTarget, vec3 Up)
	{
		position = CameraPosition;
		target = CameraTarget;
		up = Up;
	}

	mat4 GetView()
	{
		return lookat(position, target, up);
	}

	void Update(GLFWwindow* window, float deltaseconds)
	{
		double xpos, ypos;

		/// ROTATE THE CAMREA WHILE RIGHT-CLICK IS HELD
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
		{
			if (bFirstPress)
			{
				/// SAVE WHERE THE CURSOR IS SO WE CAN PUT IT BACK WHEN THEY RELEASE THE BUTTON
				glfwGetCursorPos(window, &xOldPos, &yOldPos);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				glfwSetCursorPos(window, 300, 300);
				bFirstPress = false;
			}
			glfwGetCursorPos(window, &xpos, &ypos);

			horizontalAngle += deltaseconds * mouseSpeed * (300 - xpos);
			verticalAngle += deltaseconds * mouseSpeed * (300 - ypos);
			verticalAngle = verticalAngle > verticalLimit ? verticalLimit : verticalAngle;
			verticalAngle = verticalAngle < -verticalLimit ? -verticalLimit : verticalAngle;

			glfwSetCursorPos(window, 300, 300);
		}
		else
		{
			if (!bFirstPress)
			{
				/// IF THE RIGHT MOUSE BUTTON IS RELEASED PUT THE CURSOR BACK WHERE IT WAS WHEN IT WAS FIRST PRESSED
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				glfwSetCursorPos(window, xOldPos, yOldPos);
			}
			bFirstPress = true;
		}

		/// MOVE THE CAMERA POSITION
		bool bPressLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
		bool bPressRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
		bool bPressForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
		bool bPressBack = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
		bool bPressUp = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
		bool bPressDown = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

		position += normalize(target - position) * deltaseconds * speed * (int(bPressForward) - int(bPressBack));
		position += normalize(cross(normalize(target - position), vec3(0, 1, 0))) * deltaseconds * speed * (int(bPressRight) - int(bPressLeft));
		position.y += deltaseconds * speed * (int(bPressUp) - int(bPressDown));

		target = position + vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));
	}
};
