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
	float mMouseSpeed = 0.2;
	double mHorizontalAngle = 3.141592;
	double mVerticalAngle = 0;
	bool bFirstPress = true;
	double xOldPos, yOldPos;

	Camera()
	{
		mPosition = vec3(0, 0, 0);
		mTarget = vec3(0, 0, 0);
		mUp = vec3(0, 1, 0);
		xOldPos = 0;
		yOldPos = 0;
	}

	Camera(vec3 CameraPosition, vec3 CameraTarget, vec3 Up)
	{
		mPosition = CameraPosition;
		mTarget = CameraTarget;
		mUp = Up;
		xOldPos = 0;
		yOldPos = 0;
	}

	mat4 GetView()
	{
		return lookat(mPosition, mTarget, mUp);
	}

	void UpdateInput(GLFWwindow* window, float deltaseconds)
	{
		double xpos, ypos;

		/// ROTATE THE CAMREA WHILE RIGHT-CLICK IS HELD
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
		{
			/// CAMERA ROTATION

			if (bFirstPress)
			{
				glfwGetCursorPos(window, &xOldPos, &yOldPos);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				bFirstPress = false;
			}
			glfwGetCursorPos(window, &xpos, &ypos);

			mHorizontalAngle += deltaseconds * mMouseSpeed * (xOldPos - xpos);
			mVerticalAngle += deltaseconds * mMouseSpeed * (yOldPos - ypos);
			mVerticalAngle = mVerticalAngle > verticalLimit ? verticalLimit : mVerticalAngle;
			mVerticalAngle = mVerticalAngle < -verticalLimit ? -verticalLimit : mVerticalAngle;

			glfwSetCursorPos(window, xOldPos, yOldPos);

			/// CAMERA POSITION

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
		}
		else
		{
			if (!bFirstPress)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			bFirstPress = true;
		}

		mTarget = mPosition + vec3(cos(mVerticalAngle) * sin(mHorizontalAngle), sin(mVerticalAngle), cos(mVerticalAngle) * cos(mHorizontalAngle));
	}
};
