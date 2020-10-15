#include "DebugSystem.h"
#include "Scene.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

void DebugSystem::Init(GLFWwindow* window)
{
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init();
  ImGui::StyleColorsDark();
}

void DebugSystem::End()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
}

void DebugSystem::StartFrame(Scene& scene, float dt)
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void DebugSystem::Update(Scene& scene, float dt)
{
  ImGui::Begin("test e");
  ImGui::Text("wfjoi");
  ImGui::Button("fdjiosa");
  ImGui::End();
}

void DebugSystem::EndFrame(Scene& scene, float dt)
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  ImGui::EndFrame();
}
