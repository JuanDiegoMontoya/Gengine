#pragma once
#include <functional>

struct ConsoleStorage;

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
  void Clear();

  void Draw();

private:
  Console();
  ConsoleStorage* console;
};