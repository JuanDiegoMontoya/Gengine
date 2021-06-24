#include "Parser.h"
#include "GAssert.h"
#include "CVarInternal.h"
#include <vector>

static std::vector<std::string> split(const char* str)
{
  std::vector<std::string> result;

  do
  {
    const char* begin = str;

    while (!std::isblank(*str) && *str)
      str++;

    std::string tok(begin, str);
    if (std::find_if(tok.begin(), tok.end(), [](char c) { return !std::isblank(c); }) != tok.end())
      result.push_back(tok);
  } while (0 != *str++);

  return result;
}

static float stof_nothrow(const std::string& str, size_t* idx)
{
  try
  {
    return std::stof(str, idx);
  }
  catch (std::invalid_argument)
  {
    return 0.0f;
  }
}

static double stod_nothrow(const std::string& str, size_t* idx)
{
  try
  {
    return std::stod(str, idx);
  }
  catch (std::invalid_argument)
  {
    return 0.0;
  }
}

enum class CmdType
{
  INVALID,
  FLOAT,
  STRING,
  IDENTIFIER, // command or cvar identifier
  VEC3,
};

CmdParser::CmdParser(const char* command)
  : cmd(command)
{
  // trim whitespace from beginning and end of string
  cmd.erase(0, cmd.find_first_not_of(" \n\r\t"));
  cmd.erase(cmd.find_last_not_of(" \n\r\t") + 1);
}

// world's worst type parser
// probably better than nothing
CmdAtom CmdParser::NextAtom()
{
  if (!Valid())
  {
    return ParseError{ .where = current, .what = "Empty command" };
  }

  CmdType type = CmdType::INVALID;

  // determine type by first character
  if (cmd[current] == '"')
  {
    type = CmdType::STRING;
    current++;
  }
  else if (cmd[current] == '{' || cmd[current] == '[' || cmd[current] == '(')
  {
    type = CmdType::VEC3;
    current++;
  }
  else if (std::isalpha(cmd[current]) || cmd[current] == '_')
  {
    type = CmdType::IDENTIFIER;
  }
  else if (std::isdigit(cmd[current]) || cmd[current] == '-' || cmd[current] == '.')
  {
    type = CmdType::FLOAT;
  }
  else
  {
    current = cmd.size();
    return ParseError{ .where = current, .what = "Token begins with invalid character" };
  }

  std::string atom;
  bool escapeNextChar{ false };
  for (; Valid();)
  {
    char c = cmd[current];
    current++;

    if (std::isblank(c) && !(type == CmdType::STRING || type == CmdType::VEC3))
    {
      break;
    }

    if (type == CmdType::STRING)
    {
      // end string with un-escaped quotation mark
      if (c == '"' && !escapeNextChar)
      {
        break;
      }

      // unescaped backslash will escape next character, how confusing!
      if (c == '\\' && !escapeNextChar)
      {
        escapeNextChar = true;
        continue;
      }

      escapeNextChar = false;
    }

    if (type == CmdType::IDENTIFIER)
    {
      if (!(std::isalnum(c) || c == '.' || c == '_'))
      {
        current = cmd.size();
        return ParseError{ .where = current, .what = "Invalid character in identifier" };
      }
    }

    if (type == CmdType::VEC3)
    {
      if (c == '}' || c == ']' || c == ')')
      {
        break;
      }
    }

    atom.push_back(c);
  }

  // skip any extra whitespace there may be between atoms
  size_t pos = cmd.find_first_not_of(" \n\r\t", current);
  if (pos != std::string::npos)
    current = pos;
  
  switch (type)
  {
  case CmdType::FLOAT:
  {
    size_t read{};
    double f = stod_nothrow(atom, &read);
    if (read != atom.size())
    {
      return ParseError{ .where = current, .what = "Failed to read float value" };
    }
    return f;
  }
  case CmdType::STRING:
    return atom;
  case CmdType::IDENTIFIER:
    return Identifier{ .name = atom };
  case CmdType::VEC3:
  {
    std::vector<std::string> tokens = split(atom.c_str());
    if (tokens.size() != 3)
    {
      return ParseError{ .where = current, .what = "Vector does not contain three tokens" };
    }

    glm::vec3 vec{};
    for (int i = 0; i < 3; i++)
    {
      size_t read{};
      vec[i] = stof_nothrow(tokens[i], &read);
      if (read != tokens[i].size())
      {
        return ParseError{ .where = current, .what = "Failed to read float value" };
      }
    }
    return vec;
  }
  default:
    return ParseError{ .where = current, .what = "Could not determine type" };
  }

  // unreachable
}
