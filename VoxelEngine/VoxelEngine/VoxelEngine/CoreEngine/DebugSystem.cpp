#include "DebugSystem.h"
#include "Scene.h"
#include <imgui/imgui.h>
//#include <imgui/imgui_impl_glfw.h>
//#include <imgui/imgui_impl_opengl3.h>
#include <Utilities/ImGuiExt.h>
#include <GLFW/glfw3.h>
#include <CoreEngine/Input.h>
#include <chrono>
#include <thread>
#include <glm/glm.hpp>

void DebugSystem::Init(GLFWwindow* win)
{
  window = win;
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
  if (activeCursor)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if (Input::IsKeyPressed(GLFW_KEY_GRAVE_ACCENT))
    activeCursor = !activeCursor;

  {
    ImGui::Begin("Graphs", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
    ImGui::PlotVar("Frametime (ms)", dt * 1000.0, 0, .05 * 1000, 240, ImVec2(300, 100));
    ImGui::End();
  }

  {
    ImGui::Begin("Stuff", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);

    static int fakeLag = 0;
    static double frameTimeExp = 0;
    static double alpha = .01;

    frameTimeExp = alpha * dt + (1.0 - alpha) * frameTimeExp;
    alpha = glm::clamp((float)dt, 0.0f, 1.0f);

    ImGui::Text("FPS: %.0f (%.1f ms)", 1.f / frameTimeExp, frameTimeExp * 1000);

    ImGui::SliderInt("Fake Lag", &fakeLag, 0, 50, "%d ms");
    if (fakeLag > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(fakeLag));

    ImGui::End();
  }
}

void DebugSystem::EndFrame(Scene& scene, float dt)
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  ImGui::EndFrame();
}
