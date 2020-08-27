/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/GraphicsSystem.h"
#include "../../Headers/Factory.h"
#include <Events/DrawEvent.h>
#include <Events/RenderEvent.h>
#include <Engine.h>
#include <Systems/Graphics/Context.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

GraphicsSystem* GraphicsSystem::pGraphicsSystem = nullptr;

void drawQuad()
{
	static unsigned int quadVAO = 0;
	static unsigned int quadVBO;
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

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
 Engine::GetEngine()->RegisterListener(this, &GraphicsSystem::RenderEventsListen);

 window = init_glfw_context();
 glfwMakeContextCurrent(window);
 glfwSwapInterval(1);
}

void GraphicsSystem::End()
{
  Engine::GetEngine()->UnregisterListener(this, &GraphicsSystem::UpdateEventsListen);
  Engine::GetEngine()->UnregisterListener(this, &GraphicsSystem::RenderEventsListen);
}

void GraphicsSystem::UpdateEventsListen(UpdateEvent* updateEvent)
{
  Engine::GetEngine()->AttachEvent(DrawEvent::GenerateDrawEvent(HANDLE_NEXT_FRAME));
  Engine::GetEngine()->AttachEvent(RenderEvent::GenerateRenderEvent(HANDLE_NEXT_FRAME));
}

void GraphicsSystem::RenderEventsListen(UpdateEvent* updateEvent)
{

  glfwSwapBuffers(window);
}
