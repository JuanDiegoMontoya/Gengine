#include "PCH.h"
#include <engine/GAssert.h>

void Assert::HandleAssert(const char* msg, const char* condition, const char* fname, long lineNumber)
{
  spdlog::critical("Assert Failed: {}\n"
    "Condition: {}\n"
    "File: {}\n"
    "Line: {}\n"
    "Terminating program.\n",
    msg, condition, fname, lineNumber);
}

