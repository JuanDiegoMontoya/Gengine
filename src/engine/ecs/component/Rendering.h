#pragma once
#include <engine/gfx/Material.h>
#include <engine/gfx/MeshUtils.h>

namespace Component
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