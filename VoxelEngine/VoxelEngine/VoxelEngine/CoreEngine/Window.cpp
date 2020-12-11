#define WIN32_LEAN_AND_MEAN
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "WindowCB.h"
#include "Window.h"

bool Window::Focus()
{
	glfwMakeContextCurrent(window);

	// MSAA
	glfwWindowHint(GLFW_SAMPLES, settings.MSAA);
	glEnable(GL_MULTISAMPLE);

	if (settings.vsync)
	{
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	}
	else
	{
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	}

	return true;
}

void Window::MakeWindow()
{
	glfwInit();
	glfwSetErrorCallback(ErrorCB);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);
#else // release mode
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif

	int numberOfMonitors = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&numberOfMonitors);
	//const GLFWvidmode* desktopMode = glfwGetVideoMode(monitors[0]);

	//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (settings.fullscreen)
	{
		layout.width = 1920;// desktopMode->width;
		layout.height = 1080;// desktopMode->height;

		layout.left = 0;
		layout.top = 0;
	}

	GLint width = layout.width == 0 ? /*desktopMode->width*/1920 : layout.width;
	GLint height = layout.height == 0 ? /*desktopMode->height*/1080 - 63 : layout.height;

	layout.width = width;
	layout.height = height;

	window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window: " << name << std::endl;
		glfwTerminate();

		WindowInitialized = false;
		return;
	}

	Focus();

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();

		WindowInitialized = false;
		return;
	}

	glfwSetWindowPos(window, layout.left, layout.top);

	glfwSetFramebufferSizeCallback(window, ViewportCB);

	glViewport(0, 0, width, height);

	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND_COLOR);

	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);

	//glDebugMessageCallback(ExErrorCB, NULL);
	//glEnable(GL_DEBUG_OUTPUT);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_PE_ERROR, GL_DEBUG_SEVERI_HIGH, 0, nullptr, GL_TRUE);
	//glClearColor(0.3f, 0.3f, 0.5f, 1.0f);

	WindowInitialized = true;

	if (settings.maximize)
	{
		glfwMaximizeWindow(window);
	}
	if (GLenum e = glGetError(); e != GL_NO_ERROR)
	{
		printf("Error in Window.h: %s\n", glewGetErrorString(e));
	}
}

Window::Window(const char* pName, Settings pSettings, Layout pLayout)
{
	name = pName;

	layout = pLayout;
	settings = pSettings;

	MakeWindow();
}

Window::~Window()
{
	if (WindowInitialized)
	{
		/*if (ImGUI Renderer Created)
		{
			ImGui_ImplOpenGL3_Shutdown();
		}*/

		glfwDestroyWindow(window);
		glfwTerminate();
	}
}