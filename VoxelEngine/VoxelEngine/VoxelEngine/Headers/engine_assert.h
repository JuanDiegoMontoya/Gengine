#pragma once

namespace Assert
{
	void HandleAssert(const char *message, const char *condition, const char *fileName, long lineNumber);
}

#ifdef DE_BUG
#define ASSERT_MSG(x, msg) do{\
		if(!(x))\
		{\
			Assert::HandleAssert(msg, #x, __FILE__, __LINE__);\
			std::terminate();\
		}\
  } while(0)

#define ASSERT(x) do{\
    if(!(x))\
		{\
      Assert::HandleAssert("Assertion Failed!", #x, __FILE__, __LINE__);\
      std::terminate();\
    }\
  } while(0)
#else
#define ASSERT(x) do { (void)sizeof(x); } while(0) //noop
#define ASSERT_MSG(x, msg) do { (void)sizeof(x), (void)sizeof(msg); } while(0)
#endif