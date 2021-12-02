#include "../PCH.h"
#include "RenderInfo.h"
#include <glad/glad.h>

namespace GFX
{
  void SetViewport(const RenderInfo& renderInfo)
  {
    glViewport(renderInfo.offset.x, renderInfo.offset.y, renderInfo.size.width, renderInfo.size.height);
  }
}