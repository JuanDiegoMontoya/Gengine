#pragma once
#include <cstdint>
#include <optional>
#include <string_view>
#include "BasicTypes.h"

namespace GFX
{
  class TextureSampler;

  struct TextureCreateInfo
  {
    ImageType imageType{};
    Format format{};
    Extent3D extent{};
    uint32_t mipLevels{};
    uint32_t arrayLayers{};
    SampleCount sampleCount{};
  };

  struct TextureUpdateInfo
  {
    UploadDimension dimension{};
    uint32_t level{};
    Extent3D offset{};
    Extent3D size{};
    UploadFormat format{};
    UploadType type{};
    void* pixels{};
  };

  // serves as the physical storage for textures
  // cannot be used directly for samplers
  class Texture
  {
  public:
    [[nodiscard]] static std::optional<Texture> Create(const TextureCreateInfo& createInfo, const std::string_view name = "");
    Texture(Texture&& old) noexcept;
    Texture& operator=(Texture&& old) noexcept;
    ~Texture();

    void SubImage(const TextureUpdateInfo& info);
    void GenMipmaps();
    [[nodiscard]] const TextureCreateInfo& CreateInfo() const { return createInfo_; }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

  private:
    friend class TextureView;
    Texture() {};
    uint32_t id_{};
    TextureCreateInfo createInfo_{};
  };



  struct TextureViewCreateInfo
  {
    ImageType viewType{};
    Format format{};
    uint32_t minLevel{};
    uint32_t numLevels{};
    uint32_t minLayer{};
    uint32_t numLayers{};
  };

  // serves as lightweight view of an image, cheap to construct, copy, and meant to be passed around
  class TextureView
  {
  public:
    // make a texture view with explicit parameters
    [[nodiscard]] static std::optional<TextureView> Create(const TextureViewCreateInfo& createInfo, const Texture& texture, const std::string_view name = "");

    // make a texture view with automatic parameters (view of whole texture, same type)
    [[nodiscard]] static std::optional<TextureView> Create(const Texture& texture, const std::string_view name = "");

    TextureView(const TextureView& other);
    TextureView(TextureView&& old) noexcept;
    TextureView& operator=(const TextureView& other);
    TextureView& operator=(TextureView&& old) noexcept;
    ~TextureView();

    void Bind(uint32_t slot, const TextureSampler& sampler);
    void Unbind(uint32_t slot); // unfortunate, but necessary to prevent state leakage until everything is upgraded
    void SubImage(const TextureUpdateInfo& info);

  private:
    friend class Framebuffer;
    static std::optional<TextureView> Create(const TextureViewCreateInfo& createInfo, uint32_t texture, Extent3D extent, const std::string_view name = "");
    TextureView() {};
    uint32_t id_{};
    TextureViewCreateInfo createInfo_{};
    Extent3D extent{};
  };

  struct SamplerState
  {
    SamplerState() {};
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
      }asBitField{};
      uint32_t asUint32;
    };

    // TODO: maybe add these later
    //float mipLodBias{ 0 };
    //CompareOp compareOp;
    //float minLod;
    //float maxLod;
  };

  // stores texture sampling parameters
  // copy + move constructible
  class TextureSampler
  {
  public:
    [[nodiscard]] static std::optional<TextureSampler> Create(const SamplerState& initialState, const std::string_view name = "");
    TextureSampler(const TextureSampler& other);
    TextureSampler(TextureSampler&& old) noexcept;
    TextureSampler& operator=(const TextureSampler& other);
    TextureSampler& operator=(TextureSampler&& old) noexcept;
    ~TextureSampler();

    void SetState(const SamplerState& samplerState);
    [[nodiscard]] const SamplerState& GetState() const noexcept { return samplerState_; }
    [[nodiscard]] uint32_t GetAPIHandle() const { return id_; }

  private:
    friend class TextureView;
    TextureSampler() {};
    void SetState(const SamplerState& samplerState, bool force);

    uint32_t id_{};
    SamplerState samplerState_{};
  };

  // unsafe way to bind texture view and sampler using only API handles
  void BindTextureViewNative(uint32_t slot, uint32_t textureViewAPIHandle, uint32_t samplerAPIHandle);
}