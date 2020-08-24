/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/GraphicsSystem.h"
#include "../../Headers/Factory.h"
#include <Events/DrawEvent.h>
#include <Engine.h>
#include <Systems/Graphics/Context.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GraphicsSystem* GraphicsSystem::pGraphicsSystem = nullptr;

GraphicsSystem::GraphicsSystem()
{
}

GraphicsSystem::~GraphicsSystem()
{
  pGraphicsSystem = nullptr;
}

std::string GraphicsSystem::GetName()
{
  return "GraphicsSystem";
}

void GraphicsSystem::Init()
{
 Engine::GetEngine()->RegisterListener(this, &GraphicsSystem::UpdateEventsListen);

 GLFWwindow* window = init_glfw_context();
 glfwMakeContextCurrent(window);
 glfwSwapInterval(1);
}

void GraphicsSystem::End()
{
}

void GraphicsSystem::UpdateEventsListen(UpdateEvent* updateEvent)
{
  Engine::GetEngine()->AttachEvent(DrawEvent::GenerateDrawEvent(HANDLE_NEXT_FRAME));
}