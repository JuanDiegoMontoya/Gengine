/*HEADER_GOES_HERE*/
#include "../../Headers/Managers/GraphicsManager.h"
#include "../../Headers/Factory.h"
#include <Events/UpdateEvent.h>
#include <Events/DrawEvent.h>
#include <Events/RenderEvent.h>
#include <Engine.h>
#include <Graphics/Context.h>

#include <Graphics/GraphicsIncludes.h>
#include <ImGuiIncludes.h>
#include <Graphics/GlobalRendererInfo.h>
#include <Systems/Camera.h>
#include <Graphics/shader.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

GraphicsManager* GraphicsManager::pGraphicsManager = nullptr;

#include <functional>
void DrawCube()
{
	static VAO* vao = nullptr;
	static VBO* vbo = nullptr;
	if (vao == nullptr)
	{
		vao = new VAO();
		vbo = new VBO(Vertices::cube_norm_tex, sizeof(Vertices::cube_norm_tex));
		VBOlayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);
		vao->AddBuffer(*vbo, layout);
	}
	vao->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

GraphicsManager::GraphicsManager()
{
}

GraphicsManager::~GraphicsManager()
{
  pGraphicsManager = nullptr;
}

std::string GraphicsManager::GetName()
{
  return "GraphicsManager";
}

void GraphicsManager::Init()
{
	Engine::GetEngine()->RegisterListener(this, &GraphicsManager::UpdateEventsListen);
	Engine::GetEngine()->RegisterListener(this, &GraphicsManager::RenderEventsListen);

	window = init_glfw_context();
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);


	Shader::shaders["flat_color"].emplace(Shader("flat_color.vs", "flat_color.fs"));

	// Initialize Dear ImGui
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();

	info = std::make_unique<GlobalRendererInfo>();
	info->window = window;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GraphicsManager::End()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
  Engine::GetEngine()->UnregisterListener(this, &GraphicsManager::UpdateEventsListen);
  Engine::GetEngine()->UnregisterListener(this, &GraphicsManager::RenderEventsListen);
}

void GraphicsManager::UpdateEventsListen(UpdateEvent* updateEvent)
{
	// Begin ImGui frame
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();

  Engine::GetEngine()->AttachEvent(DrawEvent::GenerateDrawEvent(HANDLE_NEXT_FRAME));
  Engine::GetEngine()->AttachEvent(RenderEvent::GenerateRenderEvent(HANDLE_NEXT_FRAME));

	/*
	// this is how I want the simple renderer to be like
	for (auto space : spaces)
	{
		for (auto camera : space->registry)
		{
			if (camera.Active())
				info->SetActiveCamera(camera);
		}

		for (auto [transform, model,...] : space->registry)
		{
			this->renderDatShit(transform, model, ...);
		}

		// VoxelWorld class will be a system rather than a system
		for (auto [voxelWorld] : space->registry)
		{
			voxelWorld.Draw();
		}
	}
	*/
	// TODO: iterate all cameras (in the space to render), set active camera here

}

void GraphicsManager::RenderEventsListen(UpdateEvent* updateEvent)
{
	return;
	auto dt = updateEvent->elapsedTime;
	glClearColor(cos(dt), sin(dt), 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto& shader = Shader::shaders["flat_color"];
	shader->Use();

	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1920.f / 1080.f, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
	glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(2 * cos(dt), 0, 2 * sin(dt)));

	shader->setMat4("u_proj", proj);
	shader->setMat4("u_view", view);
	shader->setMat4("u_model", model);
	shader->setVec4("u_color", glm::vec4(1, 1, 1, 1));
	DrawCube();

	ImGui::Begin("Test");
	ImGui::Text("dofasodf");
	ImGui::End();

	// End ImGui frame
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	ImGui::EndFrame();

  glfwSwapBuffers(window);
}