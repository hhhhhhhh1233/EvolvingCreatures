#pragma once
#include "core/math/mat4.h"
#include "GLFW/glfw3.h"

class Input
{
public:
	const float cubeSpeed = 0.05f;
	const float cameraSpeed = 0.05f;

	vec3 translation;
	double lastX, lastY;
	
	float rightClickDeltaX, rightClickDeltaY;
	bool initialRightClick;
	double rightClickReleaseX, rightClickReleaseY;

	Input()
	{
		translation = vec3();
		lastX = 0, lastY = 0;
		
		rightClickDeltaX = 0; rightClickDeltaY = 0;
		rightClickReleaseX = 0, rightClickReleaseY = 0;
		initialRightClick = true;
	}

	void HandleCam(GLFWwindow* handle, Camera& cam)
	{
		// Gets cursor position and changes the cameras rotation based on that
		double xpos, ypos;
		glfwGetCursorPos(handle, &xpos, &ypos);
		cam.Movement((xpos - lastX) - rightClickDeltaX, (lastY - ypos) - rightClickDeltaY);

		// Changes the cameras position based on input from WASD and cameraSpeed constant
		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
			cam.position += (cam.target - cam.position) * cameraSpeed;
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
			cam.position += cross((cam.target - cam.position), vec3(0, 1, 0)) * -cameraSpeed;
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
			cam.position += (cam.target - cam.position) * -cameraSpeed;
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
			cam.position += cross((cam.target - cam.position), vec3(0, 1, 0)) * cameraSpeed;

		// Sets the last loop variables so next time this is called we only rotate by an offset
		lastY = ypos;
		lastX = xpos;
	}

	void FreeCamOnRightClick(GLFWwindow* handle, Camera& cam)
	{

		// If the right mouse button is pressed rotate and move the camera around
		if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			// Calculates the difference in how far 
			if (initialRightClick)
			{
				double x, y;
				glfwGetCursorPos(handle, &x, &y);
				rightClickDeltaX = x - rightClickReleaseX;
				rightClickDeltaY = rightClickReleaseY - y;
				initialRightClick = false;
			}
			HandleCam(handle, cam);
			rightClickDeltaX = 0;
			rightClickDeltaY = 0;
		}

		// Logs the position that the cursor was at when the right mouse button is released
		if (glfwGetMouseButton(handle, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE)
		{
			if (!initialRightClick)
			{
				glfwGetCursorPos(handle, &rightClickReleaseX, &rightClickReleaseY);
				initialRightClick = true;
			}
		}
	}

	mat4 HandleCube(GLFWwindow* handle, Camera cam)
	{
		// Modifies the translation matrix based on input from WASD keys and the cubeSpeed constant, moving the cube from the cameras perspective
		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
			translation += (cam.target - cam.position) * cubeSpeed;
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
			translation -= cross((cam.target - cam.position), vec3(0,1,0)) * cubeSpeed;
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
			translation -= (cam.target - cam.position) * cubeSpeed;
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
			translation += cross((cam.target - cam.position), vec3(0,1,0)) * cubeSpeed;

		// Creates a rotation matrix based on input
		double xpos, ypos;
		glfwGetCursorPos(handle, &xpos, &ypos);
		mat4 rotation = rotationy(0.01f * (xpos)) * rotationx(0.01f * (ypos));
		
		// Combines the translation matrix and rotation matrix and return them
		return translate(translation) * rotation;
	}
};