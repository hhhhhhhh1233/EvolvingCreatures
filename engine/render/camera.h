#pragma once
#include "core/math/mat4.h"

// For easier camera handling and encapsulation
class Camera
{
public:
	/// BASIC CAMERA PROPERTIES
	vec3 mPosition;
	vec3 mTarget;
	vec3 mUp;

	/// CONTROLLABLE CAMERA MOVEMENT PROPERTIES
	const float REGULAR_SPEED = 10;
	const float INCREASED_SPEED = 30;

	const float verticalLimit = 3.14/2; /// How many radians up or down the camera can move
	float mCurrentSpeed = REGULAR_SPEED;
	float mMouseSpeed = 0.5;
	double mHorizontalAngle = 3.141592;
	double mVerticalAngle = 0;
	bool bFirstPress = true;
	double xOldPos, yOldPos;


	Camera()
	{
		mPosition = vec3(0, 0, 0);
		mTarget = vec3(0, 0, 0);
		mUp = vec3(0, 1, 0);
	}

	Camera(vec3 CameraPosition, vec3 CameraTarget, vec3 Up)
	{
		mPosition = CameraPosition;
		mTarget = CameraTarget;
		mUp = Up;
	}

	mat4 GetView()
	{
		return lookat(mPosition, mTarget, mUp);
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

			mHorizontalAngle += deltaseconds * mMouseSpeed * (300 - xpos);
			mVerticalAngle += deltaseconds * mMouseSpeed * (300 - ypos);
			mVerticalAngle = mVerticalAngle > verticalLimit ? verticalLimit : mVerticalAngle;
			mVerticalAngle = mVerticalAngle < -verticalLimit ? -verticalLimit : mVerticalAngle;

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

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			mCurrentSpeed = INCREASED_SPEED;
		else
			mCurrentSpeed = REGULAR_SPEED;

		/// MOVE THE CAMERA POSITION
		bool bPressLeft = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
		bool bPressRight = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
		bool bPressForward = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
		bool bPressBack = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
		bool bPressUp = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
		bool bPressDown = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;

		mPosition += normalize(mTarget - mPosition) * deltaseconds * mCurrentSpeed * (int(bPressForward) - int(bPressBack));
		mPosition += normalize(cross(normalize(mTarget - mPosition), vec3(0, 1, 0))) * deltaseconds * mCurrentSpeed * (int(bPressRight) - int(bPressLeft));
		mPosition.y += deltaseconds * mCurrentSpeed * (int(bPressUp) - int(bPressDown));

		if (mPosition.y < 0.2)
			mPosition.y = 0.2;

		mTarget = mPosition + vec3(cos(mVerticalAngle) * sin(mHorizontalAngle), sin(mVerticalAngle), cos(mVerticalAngle) * cos(mHorizontalAngle));
	}
};
