#pragma once
//#include <variant>

struct ConsoleStorage;

using ConsoleFunc = void(*)(const char*);

class Console
{
public:
  static Console* Get();
  ~Console();

  void Log(const char* format, ...);
  void ExecuteCommand(const char* cmd);
  void Draw();

private:
  Console();
  ConsoleStorage* console;
};