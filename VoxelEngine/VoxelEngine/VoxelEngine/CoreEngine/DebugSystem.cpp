#include "PCH.h"
#include "DebugSystem.h"
#include "Scene.h"
#include "Console.h"
#include "Engine.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <Utilities/ImGuiExt.h>
#include <CoreEngine/Input.h>
#include <chrono>
#include <thread>

void DebugSystem::Init(GLFWwindow* win)
{
  window = win;
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init();
  ImGui::StyleColorsDark();
  Input::window_ = window;
}

void DebugSystem::End()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
}

void DebugSystem::StartFrame(Scene& scene, [[maybe_unused]] float dt)
{
  if (glfwWindowShouldClose(window))
  {
    scene.GetEngine().Stop();
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void DebugSystem::Update([[maybe_unused]] Scene& scene, float dt)
{
  {
    ImGui::Begin("Graphs");
    ImGui::PlotVar("Frametime (ms)", dt * 1000.0f, 0.f, .05f * 1000.f, 240, ImVec2(300, 100));
    ImGui::End();
  }

  {
    ImGui::Begin("Stuff");

    static int fakeLag = 0;
    static double frameTimeExp = 0;
    static double alpha = .01;

    frameTimeExp = alpha * dt + (1.0 - alpha) * frameTimeExp;
    alpha = glm::clamp((float)dt, 0.0f, 1.0f);

    ImGui::Text("FPS: %.0f (%.2f ms)", 1.f / frameTimeExp, frameTimeExp * 1000);

    ImGui::SliderInt("Fake Lag", &fakeLag, 0, 50, "%d ms");
    if (fakeLag > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(fakeLag));
    }

    ImGui::End();
  }
}

void DebugSystem::EndFrame([[maybe_unused]] Scene& scene, [[maybe_unused]] float dt)
{
  static bool ting = false;
  if (Input::IsKeyPressed(GLFW_KEY_F1))
    ting = !ting;
  if (Input::GetCursorVisible())
  {
    Console::Get()->Draw();
    if (ting)
    {
      ImGui::ShowDemoWindow(&ting);
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
  ImGui::EndFrame();
}
