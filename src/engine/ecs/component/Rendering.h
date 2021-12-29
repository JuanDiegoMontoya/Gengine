#pragma once
#include <engine/gfx/Material.h>
#include <engine/gfx/Mesh.h>

namespace Component
{
  struct BatchedMesh
  {
    GFX::MeshID handle;
  };

  struct Material
  {
    GFX::MaterialID handle;
  };
}