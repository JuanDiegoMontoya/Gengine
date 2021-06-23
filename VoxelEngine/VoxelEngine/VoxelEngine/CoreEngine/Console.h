#pragma once
#include <functional>

struct ConsoleStorage;

using ConsoleFunc = std::function<void(const char*)>;

struct CColor
{
  union
  {
    float val[3];
    struct
    {
      float r;
      float g;
      float b;
    };
  };
};

class Console
{
public:
  static Console* Get();
  ~Console();

  void RegisterCommand(const char* name, const char* description, ConsoleFunc fn);
  void ExecuteCommand(const char* cmd);
  const char* GetCommandDesc(const char* name);

  void Log(const char* format, ...);
  void LogColor(const CColor& color, const char* format, ...);
  void Clear();

  void Draw();

private:
  Console();
  void DrawWindow();
  void DrawPopup();
  ConsoleStorage* console;
};