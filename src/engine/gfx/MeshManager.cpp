#include "../PCH.h"
#include "MeshManager.h"

namespace GFX
{
  struct MeshManagerStorage
  {

  };

  MeshManager* MeshManager::Get()
  {
    static MeshManager manager;
    return &manager;
  }

  MeshManager::MeshManager()
  {
    storage = new MeshManagerStorage;
  }

  MeshManager::~MeshManager()
  {
    delete storage;
  }
}