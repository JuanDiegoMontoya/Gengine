#include "PCH.h"
#include "DebugMarker.h"
#include <GL/glew.h>

namespace GFX
{
  DebugMarker::DebugMarker(const char* message)
  {
    glPushDebugGroup(
      GL_DEBUG_SOURCE_APPLICATION,
      0,
      -1,
      message);
  }

  DebugMarker::~DebugMarker()
  {
    glPopDebugGroup();
  }
}