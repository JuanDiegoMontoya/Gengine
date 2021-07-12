#pragma once
#include <utility/serialize.h>
#include <voxel/light.h>
#include <vector>

namespace Voxels
{
  enum class Visibility
  {
    Opaque = 0,
    Partial = 1,
    Invisible = 2
  };

  // visual properties (for now)
  struct BlockProperties
  {
    BlockProperties(
      const char* n,
      glm::uvec4 e,
      int pr,
      float time = 0.5f,
      bool des = true,
      Visibility vis = Visibility::Opaque,
      const char* tx = "<null>")
      : name(n), emittance(e), priority(pr), ttk(time), destructible(des), visibility(vis), texture(tx)
    {
    }
    const char* name;
    glm::u8vec4 emittance; // light
    int priority;
    float ttk;
    bool destructible;
    Visibility visibility; // skip rendering if true
    const char* texture;   // path to texture (default to name)
  };


  // defines various block properties and behaviors
  enum class BlockType : uint16_t // upgrade when over 2^16 block types
  {
    bAir = 0, // default type
    bStone,
    bDirt,
    bMetal,
    bGrass,
    bSand,
    bSnow,
    bWater,
    bOakWood,
    bOakLeaves,
    bError,
    bDryGrass,
    bOLight,
    bRLight,
    bGLight,
    bBLight,
    bSmLight,
    bYLight,
    bRglass,
    bGglass,
    bBglass,
    bDevValue100,
    bDevValue90,
    bDevValue80,
    bDevValue70,
    bDevValue60,
    bDevValue50,
    bDevValue40,
    bDevValue30,
    bDevValue20,
    bDevValue10,
    bDevValue00,

    bCount
  };


  struct Block
  {
  public:

    Block(BlockType t = BlockType::bAir) : type_(t) {}
    Block(BlockType t, Light l) : type_(t), light_(l) {}

    // Getters
    BlockType GetType() const { return type_; }
    int GetTypei() const { return static_cast<int>(type_); }
    const char* GetName() const { return Block::PropertiesTable[GetTypei()].name; }
    int GetPriority() const { return Block::PropertiesTable[GetTypei()].priority; }
    float GetTTK() const { return Block::PropertiesTable[GetTypei()].ttk; }
    bool GetDestructible() const { return Block::PropertiesTable[GetTypei()].destructible; }
    glm::u8vec4 GetEmittance() const { return Block::PropertiesTable[GetTypei()].emittance; }
    Visibility GetVisibility() const { return Block::PropertiesTable[GetTypei()].visibility; }
    Light& GetLightRef() { return light_; }
    const Light& GetLightRef() const { return light_; }
    Light GetLight() const { return light_; }

    // Setters
    void SetType(BlockType ty) { type_ = ty; }

    // Serialization
    template <class Archive>
    void serialize(Archive& ar)
    {
      uint8_t fake;
      ar(type_, fake);
    }

    static const std::vector<BlockProperties> PropertiesTable;

  private:
    BlockType type_; // could probably shove extra data in this
    Light light_;
  };
}