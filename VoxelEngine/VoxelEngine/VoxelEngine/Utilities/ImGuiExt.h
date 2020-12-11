#pragma once
#include <float.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

namespace ImGui
{
  // Plot value over time
  // Pass FLT_MAX value to draw without adding a new value
  void PlotVar(const char* label, float value, 
    float scale_min = FLT_MAX, float scale_max = FLT_MAX, 
    size_t buffer_size = 120, ImVec2 graph_size = ImVec2(0, 40));

  // Call this periodically to discard old/unused data
  void PlotVarFlushOldEntries();
}