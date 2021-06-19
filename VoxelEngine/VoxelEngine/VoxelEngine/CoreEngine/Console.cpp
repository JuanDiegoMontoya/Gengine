#include "Console.h"
#include "CVarInternal.h"
#include "Input.h"
#include <array>
#include <vector>
#include <imgui/imgui.h>


int TextEditCallback(ImGuiInputTextCallbackData* data, ConsoleStorage* console);

struct Command
{
  std::string name;
  std::string description; // printed when `help` is executed on it
  ConsoleFunc func{};
};

struct ConsoleStorage
{
  bool isOpen = true;
  std::vector<std::string> logEntries;
  std::vector<std::string> inputHistory;
  int historyPos{ -1 };
  std::array<char, 256> inputBuffer{ 0 };
  std::vector<Command> commands;
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

void Console::RegisterCommand(const char* name, const char* description, ConsoleFunc fn)
{
  console->commands.emplace_back(name, description, fn);
}

void Console::Log(const char* format, ...)
{
  // FIXME-OPT
  std::array<char, 1024> buf;
  va_list args;
  va_start(args, format);
  vsnprintf(buf.data(), buf.size(), format, args);
  buf[buf.size() - 1] = 0;
  va_end(args);
  console->logEntries.push_back(buf.data());
}

void Console::ExecuteCommand(const char* cmd)
{
  Log("# %s\n", cmd);

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
}

void Console::Draw()
{
  if (ImGui::IsKeyPressed(GLFW_KEY_F2))
    console->isOpen = !console->isOpen;

  if (!console->isOpen)
    return;

  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("GConsole", &console->isOpen, ImGuiWindowFlags_MenuBar))
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
    if (ImGui::Selectable("Clear")) console->logEntries.clear();
    ImGui::EndPopup();
  }

  // display all the entries in the console
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
  for (int i = 0; i < console->logEntries.size(); i++)
  {
    const char* item = console->logEntries[i].c_str();
    //if (!Filter.PassFilter(item))
    //  continue;

    // Normally you would store more information in your item than just a string.
    // (e.g. make Items[] an array of structure, store color/type etc.)
    ImVec4 color;
    bool has_color = false;
    if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
    else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
    if (has_color)
      ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(item);
    if (has_color)
      ImGui::PopStyleColor();
  }

  //if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
  //  ImGui::SetScrollHereY(1.0f);
  //ScrollToBottom = false;

  ImGui::PopStyleVar();
  ImGui::EndChild();
  ImGui::Separator();

  auto textEditCallbackStub = [](ImGuiInputTextCallbackData* data) -> int
    {
      ConsoleStorage* cc = reinterpret_cast<ConsoleStorage*>(data->UserData);
      return TextEditCallback(data, cc);
    };

  // Command-line
  bool reclaim_focus = false;
  ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
  if (ImGui::InputText("Input", console->inputBuffer.data(), console->inputBuffer.size(), input_text_flags, textEditCallbackStub, (void*)console))
  {
    std::string s = console->inputBuffer.data();
    //Strtrim(s);
    s.erase(s.find_last_not_of(" \n\r\t") + 1); // trim whitespace from end of string
    if (s[0])
      ExecuteCommand(s.c_str());
    //strcpy(s, "");
    console->inputBuffer[0] = NULL;
    reclaim_focus = true;
  }

  // Auto-focus on window apparition
  ImGui::SetItemDefaultFocus();
  if (reclaim_focus)
    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

  ImGui::End();
}

int TextEditCallback(ImGuiInputTextCallbackData* data, ConsoleStorage* console)
{
  //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
  switch (data->EventFlag)
  {
  case ImGuiInputTextFlags_CallbackCompletion:
  {
    // Locate beginning of current word
    const char* word_end = data->Buf + data->CursorPos;
    const char* word_start = word_end;
    while (word_start > data->Buf)
    {
      const char c = word_start[-1];
      if (c == ' ' || c == '\t' || c == ',' || c == ';')
        break;
      word_start--;
    }

    // Build a list of candidates
    std::vector<const char*> candidates;
    for (int i = 0; i < console->commands.size(); i++)
      if (strncmp(console->commands[i].name.c_str(), word_start, (int)(word_end - word_start)) == 0)
        candidates.push_back(console->commands[i].name.c_str());

    if (candidates.empty())
    {
      // No match
      Console::Get()->Log("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
    }
    else if (candidates.size() == 1)
    {
      // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
      data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
      data->InsertChars(data->CursorPos, candidates[0]);
      data->InsertChars(data->CursorPos, " ");
    }
    else
    {
      // Multiple matches. Complete as much as we can..
      // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
      int match_len = (int)(word_end - word_start);
      for (;;)
      {
        int c = 0;
        bool all_candidates_matches = true;
        for (int i = 0; i < candidates.size() && all_candidates_matches; i++)
          if (i == 0)
            c = toupper(candidates[i][match_len]);
          else if (c == 0 || c != toupper(candidates[i][match_len]))
            all_candidates_matches = false;
        if (!all_candidates_matches)
          break;
        match_len++;
      }

      if (match_len > 0)
      {
        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
      }

      // List matches
      Console::Get()->Log("Possible matches:\n");
      for (int i = 0; i < candidates.size(); i++)
        Console::Get()->Log("- %s\n", candidates[i]);
    }

    break;
  }
  case ImGuiInputTextFlags_CallbackHistory:
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

    // A better implementation would preserve the data on the current input line along with cursor position.
    if (prev_history_pos != console->historyPos)
    {
      const char* history_str = (console->historyPos >= 0) ? console->inputHistory[console->historyPos].c_str() : "";
      data->DeleteChars(0, data->BufTextLen);
      data->InsertChars(0, history_str);
    }
  }
  }
  return 0;
}