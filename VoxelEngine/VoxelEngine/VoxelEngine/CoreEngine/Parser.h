#pragma once
#include <string>
#include <variant>
#include "CVar.h"

struct ParseError
{
  size_t where{};
  std::string what;
};

struct Identifier
{
  std::string name;
};

using CmdAtom = std::variant<ParseError, Identifier, cvar_float, std::string>;

class CmdParser
{
public:
  CmdParser(const char* command);

  CmdAtom NextAtom();

  bool Valid() const noexcept
  {
    return current < cmd.size();
  }

  std::string GetRemaining() const
  {
    return cmd.c_str() + current;
  }

private:
  size_t current{ 0 };
  std::string cmd; // TODO: use const char*
};