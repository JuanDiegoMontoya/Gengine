#include <iostream>

#include "WindowCB.h"
#include "Window.h"
#include <GL/glew.h>

void ViewportCB(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	Window::layout.width = width;
	Window::layout.height = height;

	Window::layout.dirty = true;

	//WindowManager[window]->rLayout().width = width;
	//WindowManager[window]->rLayout().height = height;
	//WindowManager[window]->rDirtyLayout() = true;
}

void ErrorCB(int error, char const* description)
{
	std::cerr << "GLFW error: " << description << std::endl;
}