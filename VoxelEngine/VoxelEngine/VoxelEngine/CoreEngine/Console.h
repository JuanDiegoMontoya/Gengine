#pragma once

struct ConsoleStorage;

namespace std
{
  template<typename Fn>
  class function;
}

using ConsoleFunc = std::function<void(const char*)>;

class Console
{
public:
  static Console* Get();
  ~Console();

  void RegisterCommand(const char* name, const char* description, ConsoleFunc fn);
  void ExecuteCommand(const char* cmd);
  const char* GetCommandDesc(const char* name);

  void Log(const char* format, ...);
  void LogColor(float r, float g, float b, const char* format, ...);
  void Clear();

  void Draw();

private:
  Console();
  void DrawWindow();
  void DrawPopup();
  ConsoleStorage* console;
};