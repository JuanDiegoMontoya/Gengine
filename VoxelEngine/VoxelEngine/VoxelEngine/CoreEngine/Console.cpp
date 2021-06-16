#include "Console.h"
#include "CVar.h"
#include "CVarInternal.h"
#include <imgui/imgui.h>
#include <queue>

struct ConsoleStorage
{
  bool isOpen = true;
};

Console* Console::Get()
{
  static Console console{};
  return &console;
}

Console::Console()
{
  console = new ConsoleStorage;
}

Console::~Console()
{
  delete console;
}

void Console::Draw()
{
  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("GConsole", &console->isOpen))
  {
    console->isOpen = false;
    ImGui::End();
    return;
  }
  console->isOpen = true;

  if (ImGui::BeginPopupContextItem())
  {
    if (ImGui::MenuItem("Close Console"))
    {
      console->isOpen = false;
    }
    ImGui::EndPopup();
  }

  ImGui::TextWrapped(
    "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
    "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
  ImGui::TextWrapped("Enter 'HELP' for help.");

  ImGui::End();
}
