#include "Console.h"
#include "CVarInternal.h"
#include "Parser.h"
#include "Input.h"
#include <array>
#include <vector>
#include <imgui/imgui.h>
#include <functional>

#define INPUT_BUF_SIZE 256

// string utilities
static std::string lower(const char* str)
{
  std::string s = str;
  for (char& c : s) c = std::tolower(c);
  return s;
}

static void trimEnd(std::string& str)
{
  str.erase(str.find_last_not_of(" \n\r\t") + 1);
}

static void trimStart(std::string& str)
{
  str.erase(0, str.find_first_not_of(" \n\r\t"));
}

struct CColor
{
  union
  {
    float val[3];
    struct { float r, g, b; };
  };
};

int TextEditCallback(ImGuiInputTextCallbackData* data, ConsoleStorage* console);

struct Command
{
  std::string name;
  std::string description; // printed when `help` is executed on it
  ConsoleFunc func{};
};

AutoCVar<cvar_vec3> defaultTextColor("c.textColor", "Default color of console text", cvar_vec3(1));
AutoCVar<cvar_vec3> defaultInputColor("c.inputColor", "Default color of console input", cvar_vec3(1));

struct ConsoleStorage
{
  bool isOpen = true;
  //CColor defaultInputColor{ .6, .6, .6 };
  //CColor defaultTextColor{ 1, 1, 1 };
  std::vector<std::pair<std::string, CColor>> logEntries;
  std::vector<std::string> inputHistory;
  int historyPos{ -1 };
  int autocompletePos{ 0 };
  std::array<char, INPUT_BUF_SIZE> inputBuffer{ 0 };
  std::vector<Command> commands;
  std::vector<std::string> autocompleteCandidates;

  struct PopupState
  {
    bool isPopupOpen = false;
    int  activeIdx = -1;          // Index of currently 'active' item by use of up/down keys
    int  clickedIdx = -1;         // Index of popup item clicked with the mouse
    bool selectionChanged = false;// Flag to help focus the correct item when selecting active item
    ImVec2 popupPos{};
    ImVec2 popupSize{};
    bool isWindowFocused = false;
    bool isPopupFocused = false;
    bool userTypedKey = false;
  } state;
};

Console* Console::Get()
{
  static Console console{};
  return &console;
}

Console::Console()
{
  console = new ConsoleStorage;
  RegisterCommand("find", "- Finds commands with substring", [con = console](const char* args)
    {
      CmdParser parser(args);
      CmdAtom atom = parser.NextAtom();
      Identifier* id = std::get_if<Identifier>(&atom);
      if (!id)
      {
        Console::Get()->Log("Usage: find <convarname>");
        return;
      }

      std::vector<const Command*> commands;
      std::string idLower = lower(id->name.c_str());
      for (const Command& cmd : con->commands)
      {
        std::string cmdLower = lower(cmd.name.c_str());
        if (cmdLower.find(idLower) != std::string::npos)
        {
          commands.push_back(&cmd);
        }
      }

      for (const Command* cmd : commands)
      {
        Console::Get()->Log("%-25s %s", cmd->name.c_str(), cmd->description.c_str());
      }
    });
  RegisterCommand("Lua", "- Runs the following Lua code", [](const char*) { Console::Get()->Log("Lua code :)"); });
  RegisterCommand("set", "- Sets the value of a cvar", [](const char* args)
    {
      CmdParser parser(args);
      CmdAtom atom1 = parser.NextAtom();
      auto* id = std::get_if<Identifier>(&atom1);
      if (!id || !CVarSystem::Get()->SetCVarParse(id->name.c_str(), parser.GetRemaining().c_str()))
      {
        Console::Get()->Log("Usage: set <convar> <value>");
      }
    });
  RegisterCommand("findall", "- Displays all cvars and concommands", [con = console, storage = CVarSystem::Get()->storage](const char*)
    {
      for (const auto& cmd : con->commands)
      {
        Console::Get()->Log("%-25s %s", cmd.name.c_str(), cmd.description.c_str());
      }
      for (const auto& [key, params] : storage->cvarParameters)
      {
        Console::Get()->Log("%-25s %s", params.name.c_str(), params.description.c_str());
      }
    });
}

Console::~Console()
{
  delete console;
}

void Console::RegisterCommand(const char* name, const char* description, ConsoleFunc fn)
{
  console->commands.emplace_back(name, description, fn);
}

void Console::Log(const char* format, ...)
{
  std::array<char, 1024> buf;
  va_list args;
  va_start(args, format);
  vsnprintf(buf.data(), buf.size(), format, args);
  buf.back() = NULL;
  va_end(args);
  cvar_vec3 color = defaultTextColor.Get();
  console->logEntries.push_back({ buf.data(), { color.r, color.g, color.b } });
}

void Console::LogColor(float r, float g, float b, const char* format, ...)
{
  std::array<char, 1024> buf;
  va_list args;
  va_start(args, format);
  vsnprintf(buf.data(), buf.size(), format, args);
  buf.back() = NULL;
  va_end(args);
  console->logEntries.push_back({ buf.data(), { r, g, b } });
}

void Console::Clear()
{
  console->logEntries.clear();
}

void Console::Draw()
{
  DrawWindow();
  DrawPopup();

  if (!console->state.isWindowFocused && !console->state.isPopupFocused)
  {
    console->state.isPopupOpen = false;
  }
}

void Console::DrawWindow()
{
  if (ImGui::IsKeyPressed(GLFW_KEY_F2))
    console->isOpen = !console->isOpen;

  if (!console->isOpen)
    return;

  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
  if (console->state.isPopupOpen)
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  if (!ImGui::Begin("GConsole", &console->isOpen, windowFlags))
  {
    ImGui::End();
    return;
  }

  if (ImGui::BeginPopupContextItem())
  {
    if (ImGui::MenuItem("Close Console"))
    {
      console->isOpen = false;
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("Edit"))
    {
      if (ImGui::MenuItem("Clear", nullptr, nullptr))
      {
        printf("test");
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Reserve enough left-over height for 1 separator + 1 input text
  const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
  ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
  if (ImGui::BeginPopupContextWindow())
  {
    if (ImGui::Selectable("Clear")) Clear();
    ImGui::EndPopup();
  }

  // display all the entries in the console
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
  for (int i = 0; i < console->logEntries.size(); i++)
  {
    const auto& pair = console->logEntries[i];
    const char* item = pair.first.c_str();
    CColor c = pair.second;
    ImVec4 color{ c.r, c.g, c.b, 1 };

    //if (!Filter.PassFilter(item))
    //  continue;

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(item);
    ImGui::PopStyleColor();
  }

  //if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
  //  ImGui::SetScrollHereY(1.0f);
  //ScrollToBottom = false;

  ImGui::PopStyleVar();
  ImGui::EndChild();
  ImGui::Separator();

  // build list of candidates
  console->autocompleteCandidates.clear();
  if (console->inputBuffer[0] != 0)
  {
    std::string inputLower = lower(console->inputBuffer.data());
    trimStart(inputLower);
    trimEnd(inputLower);
    for (int i = 0; i < console->commands.size(); i++)
    {
      std::string cmdLower = lower(console->commands[i].name.c_str());
      if (cmdLower.find(inputLower) != std::string::npos)
      {
        console->autocompleteCandidates.push_back(console->commands[i].name.c_str());
      }
    }

    for (const auto& [key, val] : CVarSystem::Get()->storage->cvarParameters)
    {
      std::string cvarLower = lower(val.name.c_str());
      if (cvarLower.find(inputLower) != std::string::npos)
      {
        console->autocompleteCandidates.push_back(val.name);
      }
    }
  }
  //console->autocompletePos = std::min((int)console->autocompleteCandidates.size() - 1, console->autocompletePos);
  //console->autocompletePos = std::max(0, console->autocompletePos);

  auto textEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
  {
    ConsoleStorage* cc = reinterpret_cast<ConsoleStorage*>(data->UserData);
    return TextEditCallback(data, cc);
  };

  // Command-line
  ImGuiInputTextFlags input_text_flags =
    ImGuiInputTextFlags_EnterReturnsTrue |
    ImGuiInputTextFlags_CallbackCompletion |
    ImGuiInputTextFlags_CallbackHistory |
    ImGuiInputTextFlags_CallbackAlways |
    ImGuiInputTextFlags_CallbackEdit;

  bool reclaim_focus = false;
  if (ImGui::InputText("##Input", console->inputBuffer.data(), console->inputBuffer.size(),
    input_text_flags, textEditCallbackStub, (void*)console))
  {
    ImGui::SetKeyboardFocusHere(-1);

    if (console->state.isPopupOpen && console->state.activeIdx != -1)
    {
      const auto& selection = console->autocompleteCandidates[console->state.activeIdx];
      memcpy_s(console->inputBuffer.data(), INPUT_BUF_SIZE, selection.c_str(), selection.size());
    }
    else
    {
      std::string s = console->inputBuffer.data();
      trimEnd(s);
      if (s[0])
      {
        ExecuteCommand(s.c_str());
      }
      console->inputBuffer[0] = NULL;
    }

    reclaim_focus = true;
    console->state.isPopupOpen = false;
    console->state.activeIdx = -1;
  }

  if (console->state.clickedIdx != -1)
  {
    ImGui::SetKeyboardFocusHere(-1);
    console->state.isPopupOpen = false;
    console->state.userTypedKey = false;
  }

  // Auto-focus on window apparition
  //ImGui::SetItemDefaultFocus();
  if (reclaim_focus)
  {
    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
  }

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
    !ImGui::IsAnyItemActive() &&
    !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  {
    ImGui::SetKeyboardFocusHere(-1);
  }

  console->state.popupPos = ImGui::GetItemRectMin();
  console->state.popupSize.x = ImGui::GetItemRectSize().x - 60;
  console->state.popupSize.y = ImGui::GetTextLineHeightWithSpacing() * 6;
  console->state.popupPos.y += ImGui::GetItemRectSize().y + 10;

  console->state.isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);

  ImGui::SameLine();

  if (ImGui::Button("Submit"))
  {
    // does nothing lol
  }

  ImGui::End();
}

void Console::DrawPopup()
{
  if (!console->state.isPopupOpen)
  {
    return;
  }

  ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoSavedSettings |
    ImGuiWindowFlags_HorizontalScrollbar |
    ImGuiWindowFlags_NoFocusOnAppearing;

  ImGui::SetNextWindowPos(console->state.popupPos);
  ImGui::SetNextWindowSize(console->state.popupSize);
  ImGui::Begin("popup", nullptr, flags);
  ImGui::PushAllowKeyboardFocus(false);

  for (int i = 0; const auto& candidate : console->autocompleteCandidates)
  {
    bool isIndexActive = console->state.activeIdx == i;
    if (isIndexActive)
    {
      ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 0, 0, 1));
    }

    ImGui::PushID(i);
    if (ImGui::Selectable(candidate.c_str(), isIndexActive))
    {
      //memcpy_s(console->inputBuffer.data(), INPUT_BUF_SIZE, candidate.c_str(), candidate.size());
      console->state.clickedIdx = i;
    }
    ImGui::PopID();

    if (isIndexActive)
    {
      if (console->state.selectionChanged)
      {
        ImGui::SetScrollHereY();
        console->state.selectionChanged = false;
      }

      ImGui::PopStyleColor();
    }
    i++;
  }

  console->state.isPopupFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
  ImGui::PopAllowKeyboardFocus();
  ImGui::End();
}

void Console::ExecuteCommand(const char* cmd)
{
  const auto& color = defaultInputColor.Get();
  LogColor(color.r, color.g, color.b, ">>> %s <<<\n", cmd);

  // Insert into history. First find match and delete it so it can be pushed to the back.
  // This isn't trying to be smart or optimal.
  console->historyPos = -1;
  for (int i = console->inputHistory.size() - 1; i >= 0; i--)
  {
    if (console->inputHistory[i] == cmd)
    {
      console->inputHistory.erase(console->inputHistory.begin() + i);
      break;
    }
  }
  console->inputHistory.push_back(cmd);

  CmdParser parser(cmd);
  auto var = parser.NextAtom();
  auto* id = std::get_if<Identifier>(&var);
  if (!id)
  {
    Log("Commands must begin with an identifier\n");
    Log("%s\n", cmd);
    Log("^ not an identifier\n");
    return;
  }

  std::string cmdLower = lower(id->name.c_str());
  auto it = std::find_if(console->commands.begin(), console->commands.end(), [cmdLower](const Command& command)
    {
      return lower(command.name.c_str()) == cmdLower;
    });

  if (it != console->commands.end())
  {
    // TODO: try get cvar with id
    it->func(parser.GetRemaining().c_str());
    return;
  }

  //auto* params = CVarSystem::Get()->GetCVarParams(id->name.c_str());
  bool success = CVarSystem::Get()->SetCVarParse(id->name.c_str(), parser.GetRemaining().c_str());

  if (!success)
  {
    Log("No cvar or concommand with identifier <%s> exists\n", id->name.c_str());
  }
}

const char* Console::GetCommandDesc(const char* name)
{
  for (const auto& cmd : console->commands)
  {
    if (lower(cmd.name.c_str()) == lower(name))
    {
      return cmd.description.c_str();
    }
  }
  return nullptr;
}

int TextEditCallback(ImGuiInputTextCallbackData* data, ConsoleStorage* console)
{
  switch (data->EventFlag)
  {
  case ImGuiInputTextFlags_CallbackCompletion:
  {
    if (console->state.isPopupOpen && console->state.activeIdx != -1 && console->autocompleteCandidates.size() > 0)
    {
      const auto& str = console->autocompleteCandidates[console->state.activeIdx];
      data->DeleteChars(0, data->BufTextLen);
      data->InsertChars(0, str.c_str());
    }

    console->state.isPopupOpen = false;
    console->state.activeIdx = -1;
    console->state.clickedIdx = -1;
    break;
  }
  case ImGuiInputTextFlags_CallbackHistory:
  {
    // move autocomplete cursor if there are candidates
    if (console->state.isPopupOpen)
    {
      if (data->EventKey == ImGuiKey_DownArrow)
      {
        if (++console->state.activeIdx > console->autocompleteCandidates.size() - 1)
          console->state.activeIdx = 0;
      }
      else if (data->EventKey == ImGuiKey_UpArrow)
      {
        if (--console->state.activeIdx < 0)
          console->state.activeIdx = console->autocompleteCandidates.size() - 1;
      }
    }
    else // move history cursor if there is no autocomplete candidate
    {
      const int prev_history_pos = console->historyPos;
      if (data->EventKey == ImGuiKey_UpArrow)
      {
        if (console->historyPos == -1)
          console->historyPos = console->inputHistory.size() - 1;
        else if (console->historyPos > 0)
          console->historyPos--;
      }
      else if (data->EventKey == ImGuiKey_DownArrow)
      {
        if (console->historyPos != -1)
          if (++console->historyPos >= console->inputHistory.size())
            console->historyPos = -1;
      }

      // a better implementation would preserve the data on the current input line along with cursor position
      if (prev_history_pos != console->historyPos)
      {
        const char* history_str = (console->historyPos >= 0) ? console->inputHistory[console->historyPos].c_str() : "";
        data->DeleteChars(0, data->BufTextLen);
        data->InsertChars(0, history_str);
      }
    }
    break;
  }
  case ImGuiInputTextFlags_CallbackAlways:
  {
    if (console->state.clickedIdx != -1)
    {
      const auto& str = console->autocompleteCandidates[console->state.clickedIdx];
      data->DeleteChars(0, data->BufTextLen);
      data->InsertChars(0, str.c_str());

      console->state.isPopupOpen = false;
      console->state.activeIdx = -1;
      console->state.clickedIdx = -1;
    }

    if (console->inputBuffer[0] == NULL)
      console->state.userTypedKey = false;

    if (console->autocompleteCandidates.empty())
      console->state.isPopupOpen = false;
    else if (console->state.userTypedKey)
      console->state.isPopupOpen = true;
    break;
  }
  case ImGuiInputTextFlags_CallbackEdit:
  {
    console->state.userTypedKey = true;
    break;
  }
  }

  return 0;
}