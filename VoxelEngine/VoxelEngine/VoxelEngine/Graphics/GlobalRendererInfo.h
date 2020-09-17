#pragma once
#include <memory>

struct GlobalRendererInfo
{
	struct GLFWwindow* window;
	struct Camera* activeCamera;
};