//------------------------------------------------------------------------------
// window.cc
// (C) 2015-2020 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "window.h"
#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"

namespace Display
{

#ifdef __WIN32__
#define APICALLTYPE __stdcall
#else
#define APICALLTYPE
#endif

//------------------------------------------------------------------------------
/**
*/
static void GLAPIENTRY
GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::string msg("[OPENGL DEBUG MESSAGE] ");

	// print error severity
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		msg.append("<Low severity> ");
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		msg.append("<Medium severity> ");
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		msg.append("<High severity> ");
		break;
	}

	// append message to output
	msg.append(message);

	// print message
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		printf("Error: %s\n", msg.c_str());
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		printf("Performance issue: %s\n", msg.c_str());
		break;
	default:		// Portability, Deprecated, Other
		break;
	}
}

int32 Window::WindowCount = 0;
//------------------------------------------------------------------------------
/**
*/
Window::Window() :
	window(nullptr),
	width(1920),
	height(1080),
	title("Evolving Creatures")
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
Window::~Window()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticKeyPressCallback(GLFWwindow* win, int32 key, int32 scancode, int32 action, int32 mods)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (ImGui::IsMouseHoveringAnyWindow())
	{
		ImGui_ImplGlfwGL3_KeyCallback(win, key, scancode, action, mods);
	}
	else if (nullptr != window->keyPressCallback)
	{
		window->keyPressCallback(key, scancode, action, mods);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticMousePressCallback(GLFWwindow* win, int32 button, int32 action, int32 mods)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (ImGui::IsMouseHoveringAnyWindow())
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback(win, button, action, mods);
	}
	else if (nullptr != window->mousePressCallback)
	{
		window->mousePressCallback(button, action, mods);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticMouseMoveCallback(GLFWwindow* win, float64 x, float64 y)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (nullptr != window->mouseMoveCallback)
	{
		window->mouseMoveCallback(x, y);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticMouseEnterLeaveCallback(GLFWwindow* win, int32 mode)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (nullptr != window->mouseLeaveEnterCallback)
	{
		window->mouseLeaveEnterCallback(mode == 0);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticMouseScrollCallback(GLFWwindow* win, float64 x, float64 y)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (ImGui::IsMouseHoveringAnyWindow())
	{
		ImGui_ImplGlfwGL3_ScrollCallback(win, x, y);
	}
	else if (nullptr != window->mouseScrollCallback)
	{
		window->mouseScrollCallback(x, y);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::StaticCloseCallback(GLFWwindow* win)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	window->Close();
}

//------------------------------------------------------------------------------
/**
*/
void
Window::Resize()
{
	if (nullptr != this->window)
	{
		glfwSetWindowSize(this->window, this->width, this->height);

		// setup viewport
		glViewport(0, 0, this->width, this->height);
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::Retitle()
{
	if (nullptr != this->window)
	{
		glfwSetWindowTitle(this->window, this->title.c_str());
	}
}

//------------------------------------------------------------------------------
/**
*/
bool
Window::Open()
{
	if (Window::WindowCount == 0)
	{
		if (!glfwInit())
		{
			return false;
		}
	}

	// setup window
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 0);

#ifdef CI_TEST
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#endif

	// open window
	this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
	if (this->window == NULL)
	{
		printf("[ERROR]: Could not create GLFW window!\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(this->window);

	if (nullptr != this->window && WindowCount == 0)
	{
		GLenum res = glewInit();
		assert(res == GLEW_OK);
		if (!(GLEW_VERSION_4_0))
		{
			printf("[ERROR]: OpenGL 4.0+ is not supported on this hardware!\n");
			glfwDestroyWindow(this->window);
			this->window = nullptr;
			glfwTerminate();
			return false;
		}

		// setup debug callback
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(GLDebugCallback, NULL);
		GLuint unusedIds;
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);

		// setup stuff
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_MULTISAMPLE);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

		// setup viewport
		glViewport(0, 0, this->width, this->height);
	}


	glfwSetWindowUserPointer(this->window, this);
	glfwSetKeyCallback(this->window, Window::StaticKeyPressCallback);
	glfwSetMouseButtonCallback(this->window, Window::StaticMousePressCallback);
	glfwSetCursorPosCallback(this->window, Window::StaticMouseMoveCallback);
	glfwSetCursorEnterCallback(this->window, Window::StaticMouseEnterLeaveCallback);
	glfwSetScrollCallback(this->window, Window::StaticMouseScrollCallback);
	glfwSetWindowCloseCallback(this->window, Window::StaticCloseCallback);
	// setup imgui implementation
    ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(this->window, false);
	glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);

	// increase window count and return result
	Window::WindowCount++;
	return this->window != nullptr;
}

//------------------------------------------------------------------------------
/**
*/
void
Window::Close()
{
	if (nullptr != this->window)
	{
		glfwDestroyWindow(this->window);
	}
	this->window = nullptr;
	Window::WindowCount--;
	if (Window::WindowCount == 0)
	{
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();
	}
}

//------------------------------------------------------------------------------
/**
*/
void
Window::MakeCurrent()
{
	glfwMakeContextCurrent(this->window);
}

//------------------------------------------------------------------------------
/**
*/
void
Window::Update()
{
	glfwPollEvents();	
}

//------------------------------------------------------------------------------
/**
*/
void
Window::SwapBuffers()
{
	if (this->window)
	{
		if (nullptr != this->uiFunc)
		{
			ImGui_ImplGlfwGL3_NewFrame();
			this->uiFunc();
			ImGui::Render();
		}
		glfwSwapBuffers(this->window);
	}
}
} // namespace Display