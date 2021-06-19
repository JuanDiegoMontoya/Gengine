#pragma once
#include <string>
#include <variant>
#include "CVar.h"
//#include <glm/glm.hpp>

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
  CmdParser(const char* command)
    : cmd(command)
  {
    // trim whitespace from beginning and end of string
    cmd.erase(0, cmd.find_first_not_of(" \n\r\t"));
    cmd.erase(cmd.find_last_not_of(" \n\r\t") + 1);
  }


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