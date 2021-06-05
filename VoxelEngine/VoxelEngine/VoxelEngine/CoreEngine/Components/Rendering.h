#pragma once
#include "../Material.h"
#include "../MeshUtils.h"

namespace Components
{
  struct BatchedMesh
  {
    MeshID handle;
    uint64_t renderFlag = (uint64_t)RenderFlags::Default;
  };

  struct Material
  {
    MaterialID handle;
  };
}