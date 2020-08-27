#include <Systems/Graphics/Context.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#pragma optimize("", off)
GLFWwindow* init_glfw_context()
{
	if (!glfwInit())
		return nullptr;
	//glfwSetErrorCallback(error_cb);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);

	// MSAA
	//glfwWindowHint(GLFW_SAMPLES, Settings::Get().Graphics.multisamples);
	//glEnable(GL_MULTISAMPLE);

	// start window maximized
	glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

	// vertical sync enabled if GL_TRUE
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int window_width = 1920; //mode->width;
	int window_height = 1016; //mode->height;

	bool fullscreen = false;
	GLFWwindow* window;
	if (fullscreen)
		window = glfwCreateWindow(window_width, window_height, "Graphics Engine", glfwGetPrimaryMonitor(), NULL);
	else
		window = glfwCreateWindow(window_width, window_height, "Engine", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwSetWindowPos(window, 0, 25);
	glfwMakeContextCurrent(window);
	//set_glfw_callbacks(window);

	glewExperimental = GL_FALSE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return nullptr;
	}

	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glViewport(0, 0, window_width, window_height - 63);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND_COLOR);

	glClearColor(0.3f, 0.3f, 0.5f, 1.0f);

	return window;
}
