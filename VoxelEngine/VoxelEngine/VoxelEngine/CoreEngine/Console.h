#pragma once
//#include <variant>

struct ConsoleStorage;

using ConsoleFunc = void(*)(const char*);

class Console
{
public:
  static Console* Get();
  ~Console();

  void Print(const char* format, ...);
  void SetIsOpen(bool isOpen);
  void Draw();
  

private:
  Console();
  ConsoleStorage* console;
};