#include "../../stdafx.h"
#include "Assert.h"

void Assert::HandleAssert(const char* msg, const char* condition, const char* fname, long lineNumber)
{
	std::cerr << "Assert Failed: \"" << msg << "\"" << '\n';
	std::cerr << "Condition: " << condition << '\n';
	std::cerr << "File: " << fname << '\n';
	std::cerr << "Line: " << lineNumber << '\n';
	std::cerr << "Terminating Program." << '\n';
}

