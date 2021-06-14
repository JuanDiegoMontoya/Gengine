#pragma once
#include <variant>

struct ConsoleStorage;

class Console
{
public:
  static Console* Get();
  ~Console();

  void Draw();

private:
  Console();
  ConsoleStorage* storage;
};