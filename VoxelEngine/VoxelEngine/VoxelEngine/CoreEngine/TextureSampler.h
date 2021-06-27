#pragma once
#include <cinttypes>

namespace GFX
{
  enum class Filter : uint8_t
  {
    NEAREST,
    LINEAR,
  };

  enum class AddressMode : uint8_t
  {
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRROR_CLAMP_TO_EDGE,
  };

  enum class BorderColor : uint8_t
  {
    FLOAT_TRANSPARENT_BLACK,
    INT_TRANSPARENT_BLACK,
    FLOAT_OPAQUE_BLACK,
    INT_OPAQUE_BLACK,
    FLOAT_OPAQUE_WHITE,
    INT_OPAQUE_WHITE,
  };

  enum class Anisotropy
  {
    SAMPLES_1,
    SAMPLES_2,
    SAMPLES_4,
    SAMPLES_8,
    SAMPLES_16,
  };

  struct SamplerState
  {
    union
    {
      struct
      {
        Filter magFilter         : 1 = Filter::LINEAR;
        Filter minFilter         : 1 = Filter::LINEAR;
        Filter mipmapFilter      : 1 = Filter::LINEAR;
        AddressMode addressModeU : 3 = AddressMode::CLAMP_TO_EDGE;
        AddressMode addressModeV : 3 = AddressMode::CLAMP_TO_EDGE;
        AddressMode addressModeW : 3 = AddressMode::CLAMP_TO_EDGE;
        BorderColor borderColor  : 3 = BorderColor::INT_OPAQUE_WHITE;
        Anisotropy anisotropy    : 3 = Anisotropy::SAMPLES_1;
      }asBitField;
      uint32_t asUint32;
    };

    //float mipLodBias{ 0 };
    //CompareOp compareOp;
    //float minLod;
    //float maxLod;
  };

  class TextureSampler
  {
  public:
    TextureSampler(const SamplerState& samplerState);
    TextureSampler(TextureSampler&& old) noexcept;
    TextureSampler& operator=(TextureSampler&& old) noexcept;
    ~TextureSampler();

    void SetState(const SamplerState& samplerState);
    const SamplerState& GetState() const noexcept { return samplerState_; }

    TextureSampler(const TextureSampler&) = delete;
    TextureSampler& operator=(const TextureSampler&) = delete;

  private:
    void SetState(const SamplerState& samplerState, bool force);

    uint32_t id_{};
    SamplerState samplerState_{};
  };
}