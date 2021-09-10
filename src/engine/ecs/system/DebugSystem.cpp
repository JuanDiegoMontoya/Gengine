#include "../../PCH.h"
#include "../../Scene.h"
#include "../../Console.h"
#include "../../Engine.h"
#include "../../core/Statistics.h"
#include "DebugSystem.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <utility/ImGuiExt.h>
#include <engine/Input.h>
#include <chrono>
#include <thread>

#include <engine/core/StatMacros.h>

DECLARE_FLOAT_STAT(DrawUI_CPU, CPU)
DECLARE_FLOAT_STAT(DrawUI_GPU, GPU)

static std::string imguiConfigDir = std::string(AssetDir) + "imgui.ini";

void DebugSystem::Init(GLFWwindow* const* win)
{
  window = win;
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(*window, false);
  ImGui_ImplOpenGL3_Init();
  ImGui::GetIO().IniFilename = imguiConfigDir.c_str();
  ImGui::StyleColorsDark();
}

void DebugSystem::End()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();
}

void DebugSystem::StartFrame(Scene& scene)
{
  if (glfwWindowShouldClose(*window))
  {
    scene.GetEngine()->Stop();
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void DebugSystem::Update([[maybe_unused]] Scene& scene, Timestep timestep)
{
  {
    ImGui::Begin("Graphs");
    ImGui::PlotVar("Frametime (ms)", timestep.dt_actual * 1000.0f, 0.f, .05f * 1000.f, 240, ImVec2(300, 100));
    ImGui::End();
  }

  {
    ImGui::Begin("Stuff");

    static int fakeLag = 0;
    static double frameTimeExp = 0;
    static double alpha = .01;

    frameTimeExp = alpha * timestep.dt_actual + (1.0 - alpha) * frameTimeExp;
    alpha = glm::clamp(timestep.dt_actual, 0.0, 1.0);

    ImGui::Text("FPS: %.0f (%.2f ms)", 1.f / frameTimeExp, frameTimeExp * 1000);

    ImGui::SliderInt("Fake Lag", &fakeLag, 0, 50, "%d ms");
    if (fakeLag > 0)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(fakeLag));
    }

    ImGui::End();
  }
}

void DebugSystem::EndFrame([[maybe_unused]] Scene& scene)
{
  MEASURE_CPU_TIMER_STAT(DrawUI_CPU);
  MEASURE_GPU_TIMER_STAT(DrawUI_GPU);

  static bool ting = false;
  if (Input::IsKeyPressed(GLFW_KEY_F1))
    ting = !ting;
  if (Input::GetCursorVisible())
  {
    engine::Core::StatisticsManager::Get()->DrawUI();
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
