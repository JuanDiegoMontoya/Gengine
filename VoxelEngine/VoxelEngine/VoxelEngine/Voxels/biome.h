#pragma once
#include <Graphics/misc_utils.h>

enum class TerrainType : unsigned
{
  tNone,  // unfilled terrain (replaced with ocean or something)
  tPlains,
  tHills,
  tOcean,

  tCount
};

// TODO: see if prefab.h needs to be included instead of this
enum struct PrefabName;

// TODO: add more properties like grass/water color modifier, etc
//       sky color,
struct Biome
{
  Biome(float humid, float temp,
    TerrainType tt, BlockType bt)
    : humidity_avg(humid), temp_avg(temp), terrain(tt), surfaceCover(bt),
  surfaceFeatures(), subFeatures(), skyFeatures() {}

  Biome() : humidity_avg(), temp_avg(), terrain(), surfaceCover(), 
    surfaceFeatures(), subFeatures(), skyFeatures() {}

  std::string name; // identifier

  // when to spawn this biome
  float humidity_avg; // -1 to 1
  float temp_avg;     // -1 to 1
  TerrainType terrain;// terrain type this biome will spawn on

  // % chance and name of prefab to spawn
  BlockType surfaceCover; // sand, dirt, snow, etc.
  std::vector<std::pair<float, PrefabName>> surfaceFeatures;  // per block
  std::vector<std::pair<float, PrefabName>> subFeatures;      // per chunk
  std::vector<std::pair<float, PrefabName>> skyFeatures;      // per chunk

  // deletes these biomes when added (for custom biomes)
  std::vector<std::string> biomeOverride;

  // serialization
  template <class Archive>
  void serialize(Archive& ar)
  {
    // ar()
  }
};

// registers biomes and stuff
class BiomeManager
{
public:
  inline static std::unordered_map<std::string, Biome> biomes;

  const static Biome& GetBiome(float temp, float humid, TerrainType terrain);
  static void InitializeBiomes();
private:
  static void registerBiome(const Biome& biome);

  static Biome loadBiome(std::string name);
  static void initCustomBiomes();
};