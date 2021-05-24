#include "prefab.h"
#include <filesystem>

#include <fstream>
#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/archives/binary.hpp>

namespace Voxels
{
  void PrefabManager::InitPrefabs()
  {
    // add basic tree prefab to list
    Prefab tree;
    for (int i = 0; i < 5; i++)
    {
      tree.Add({ 0, i, 0 }, Block(BlockType::bOakWood));

      if (i > 2)
      {
        tree.Add({ -1, i, 0 }, Block(BlockType::bOakLeaves));
        tree.Add({ +1, i, 0 }, Block(BlockType::bOakLeaves));
        tree.Add({ 0, i, -1 }, Block(BlockType::bOakLeaves));
        tree.Add({ 0, i, +1 }, Block(BlockType::bOakLeaves));
      }

      if (i == 4)
        tree.Add({ 0, i + 1, 0 }, Block(BlockType::bOakLeaves));
    }
    tree.SetPlacementType(PlacementType::NoRestrictions);
    tree.name = "Oak Tree";
    prefabs_["OakTree"] = tree;

    Prefab bTree;
    for (int i = 0; i < 8; i++)
    {
      if (i < 7)
        bTree.Add({ 0, i, 0 }, Block(BlockType::bOakWood));
      else
        bTree.Add({ 0, i, 0 }, Block(BlockType::bOakLeaves));

      if (i > 4)
      {
        bTree.Add({ -1, i, 0 }, Block(BlockType::bOakLeaves));
        bTree.Add({ +1, i, 0 }, Block(BlockType::bOakLeaves));
        bTree.Add({ 0 , i, -1 }, Block(BlockType::bOakLeaves));
        bTree.Add({ 0 , i, +1 }, Block(BlockType::bOakLeaves));

        bTree.Add({ -1, i, -1 }, Block(BlockType::bOakLeaves));
        bTree.Add({ +1, i, +1 }, Block(BlockType::bOakLeaves));
        bTree.Add({ +1, i, -1 }, Block(BlockType::bOakLeaves));
        bTree.Add({ -1, i, +1 }, Block(BlockType::bOakLeaves));
      }
    }
    bTree.SetPlacementType(PlacementType::PriorityRequired);
    bTree.name = "Oak Tree Big";
    prefabs_["OakTreeBig"] = bTree;

    // error prefab to be generated when an error occurs
    Prefab error;
    for (int x = 0; x < 3; x++)
    {
      for (int y = 0; y < 3; y++)
      {
        for (int z = 0; z < 3; z++)
        {
          error.Add({ x, y, z }, Block(BlockType::bError));
        }
      }
    }
    error.SetPlacementType(PlacementType::NoOverwriting);
    error.name = "Error";
    prefabs_["Error"] = error;

    SavePrefabToFile(error, "Error");
    //LoadAllPrefabs();
  }

  const Prefab& PrefabManager::GetPrefab(std::string p)
  {
    if (auto it = prefabs_.find(p); it != prefabs_.end())
    {
      return it->second;
    }

    // lazy
    return prefabs_[p] = LoadPrefabFromFile(p);
  }

  // TODO: add support for xml and json archives (just check the file extension)
  Prefab PrefabManager::LoadPrefabFromFile(std::string filename)
  {
    try
    {
      std::ifstream is("./Resources/Prefabs/" + filename + ".bin", std::ios::binary);
      if (is.is_open())
      {
        cereal::BinaryInputArchive archive(is);
        Prefab pfb;
        archive(pfb);
        return pfb;
      }
    }
    catch (...)
    {
      //return prefabs_[Prefab::Error];
    }
    return prefabs_["Error"];
  }

  void PrefabManager::SavePrefabToFile(const Prefab& prefab, std::string filename)
  {
    try
    {
      std::ofstream is("./Resources/Prefabs/" + filename + ".bin", std::ios::binary);
      if (is.is_open())
      {
        cereal::BinaryOutputArchive archive(is);
        archive(prefab);
      }
    }
    catch (...)
    {
      printf("Error saving prefab.\n");
    }
  }

  void PrefabManager::LoadAllPrefabs()
  {
    //prefabs_["DungeonSmall"] = LoadPrefabFromFile("dungeon");
    //prefabs_["BorealTree"] = LoadPrefabFromFile("borealTree");
    //prefabs_["Cactus"] = LoadPrefabFromFile("cactus");
    //prefabs_["BoulderA"] = LoadPrefabFromFile("boulderA");
    //prefabs_["BoulderB"] = LoadPrefabFromFile("boulderB");
    //prefabs_["BoulderC"] = LoadPrefabFromFile("boulderC");
  }
}