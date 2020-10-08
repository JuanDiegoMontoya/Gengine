#pragma once
#include <block.h>
#include <map>

enum struct PrefabName : int
{
  OakTree,
  OakTreeBig,
  Error,
  DungeonSmall,
  BorealTree,
  Cactus,
  BoulderA,
  BoulderB,
  BoulderC,

  pfCount
};

// an object designed to be pasted into the world
struct Prefab
{
  void Add(glm::ivec3 pos, Block block)
  {
    blocks.push_back(std::pair<glm::ivec3, Block>(pos, block));
  }

  // blocks and their positions relative to the spawn point of the prefab
  std::vector<std::pair<glm::ivec3, Block>> blocks;

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(blocks);
  }
};

class PrefabManager
{
public:
  static void InitPrefabs();
  static const Prefab& GetPrefab(PrefabName p) { return prefabs_[p]; }

private:
  static Prefab LoadPrefabFromFile(std::string name);
  static void LoadAllPrefabs();

  static std::map<PrefabName, Prefab> prefabs_;
};