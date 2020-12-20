#pragma once
#include <Voxels/block.h>
#include <unordered_map>
#include <Utilities/Serialize.h>

enum class PlacementType : uint16_t
{
  NoRestrictions = 0, // No prefab spawning restrictions
  PriorityRequired,   // All spawned blocks have to pass priority checks
  NoOverwriting       // No spawned blocks can overwrite pre-existing blocks
};

// an object designed to be pasted into the world
struct Prefab
{
  Prefab(PlacementType t = PlacementType::NoRestrictions) : type(t) {}

  void SetPlacementType(PlacementType placetype)
  {
    type = placetype;
  }

  PlacementType GetPlacementType()
  {
    return type;
  }

  void Add(glm::ivec3 pos, Block block)
  {
    blocks.push_back(std::pair<glm::ivec3, Block>(pos, block));
  }

  // blocks and their positions relative to the spawn point of the prefab
  std::vector<std::pair<glm::ivec3, Block>> blocks;
  PlacementType type;

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
  static const Prefab& GetPrefab(std::string p);

private:
  static Prefab LoadPrefabFromFile(std::string filename);
  static void SavePrefabToFile(const Prefab& prefab, std::string filename);
  static void LoadAllPrefabs();

  static inline std::unordered_map<std::string, Prefab> prefabs_;
};