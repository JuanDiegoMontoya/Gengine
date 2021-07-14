#pragma once
#include <optional>
#include <utility/HashedString.h>

namespace GFX
{
  class MeshManager
  {
  public:
    [[nodiscard]] static MeshManager* Get();



  private:
    MeshManager();
    ~MeshManager();

    struct MeshManagerStorage* storage;
  };
}