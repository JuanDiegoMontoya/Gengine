#pragma once
#include <cstdint>
#include <optional>

namespace GFX
{
  class TextureSampler;

  enum class ImageType : uint8_t
  {
    TEX_1D,
    TEX_2D,
    TEX_3D,
    TEX_1D_ARRAY,
    TEX_2D_ARRAY,
    TEX_CUBEMAP,
    //TEX_CUBEMAP_ARRAY, // extremely cursed- do not use
    TEX_2D_MULTISAMPLE,
    TEX_2D_MULTISAMPLE_ARRAY,
  };

  enum class Format : uint8_t
  {
    UNDEFINED,
    R8_UNORM,
    R8_SNORM,
    R16_UNORM,
    R16_SNORM,
    R8G8_UNORM,
    R8G8_SNORM,
    R16G16_UNORM,
    R16G16_SNORM,
    R3G3B2_UNORM,
    R4G4B4_UNORM,
    R5G5B5_UNORM,
    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R10G10B10_UNORM,
    R12G12B12_UNORM,
    R16G16B16_SNORM,
    R2G2B2A2_UNORM,
    R4G4B4A4_UNORM,
    R5G5B5A1_UNORM,
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R10G10B10A2_UNORM,
    R10G10B10A2_UINT,
    R12G12B12A12_UNORM,
    R16G16B16A16_UNORM,
    R8G8B8_SRGB,
    R8G8B8A8_SRGB,
    R16_FLOAT,
    R16G16_FLOAT,
    R16G16B16_FLOAT,
    R16G16B16A16_FLOAT,
    R32_FLOAT,
    R32G32_FLOAT,
    R32G32B32_FLOAT,
    R32G32B32A32_FLOAT,
    R11G11B11_FLOAT,
    R9G9B9_E5,
    R8_SINT,
    R8_UINT,
    R16_SINT,
    R16_UINT,
    R32_SINT,
    R32_UINT,
    R8G8_SINT,
    R8G8_UINT,
    R16G16_SINT,
    R16G16_UINT,
    R32G32_SINT,
    R32G32_UINT,
    R8G8B8_SINT,
    R8G8B8_UINT,
    R16G16B16_SINT,
    R16G16B16_UINT,
    R32G32B32_SINT,
    R32G32B32_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_UINT,
    // TODO: compressed formats
  };

  struct Extent3D
  {
    uint32_t width{};
    uint32_t height{};
    uint32_t depth{};
  };

  enum class SampleCount : uint8_t
  {
    ONE,
    TWO,
    FOUR,
    EIGHT,
    SIXTEEN,
  };

  enum class UploadDimension : uint8_t
  {
    ONE,
    TWO,
    THREE,
  };

  enum class UploadFormat : uint8_t
  {
    UNDEFINED,
    R,
    RG,
    RGB,
    BGR,
    RGBA,
    BGRA,
    DEPTH_COMPONENT,
    STENCIL_INDEX,
  };

  enum class UploadType : uint8_t
  {
    UNDEFINED,
    UBYTE,
    SBYTE,
    USHORT,
    SSHORT,
    UINT,
    SINT,
    FLOAT,
    UBYTE_3_3_2,
    UBYTE_2_3_3,
    USHORT_5_6_5,
    USHORT_5_6_5_REV,
    USHORT_4_4_4_4,
    USHORT_4_4_4_4_REV,
    USHORT_5_5_5_1,
    USHORT_5_5_5_1_REV,
    UINT_8_8_8_8,
    UINT_8_8_8_8_REV,
    UINT_10_10_10_2,
    UINT_10_10_10_2_REV,
  };

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

  enum class Anisotropy : uint8_t
  {
    SAMPLES_1,
    SAMPLES_2,
    SAMPLES_4,
    SAMPLES_8,
    SAMPLES_16,
  };

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

  // serves as lightweight view of an image, meant to be passed around
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
    [[nodiscard]] static std::optional<TextureSampler> Create(const SamplerState& samplerState, const std::string_view name = "");
    TextureSampler(const TextureSampler& other);
    TextureSampler(TextureSampler&& old) noexcept;
    TextureSampler& operator=(const TextureSampler& other);
    TextureSampler& operator=(TextureSampler&& old) noexcept;
    ~TextureSampler();

    void SetState(const SamplerState& samplerState);
    [[nodiscard]] const SamplerState& GetState() const noexcept { return samplerState_; }


  private:
    friend class TextureView;
    TextureSampler() {};
    void SetState(const SamplerState& samplerState, bool force);

    uint32_t id_{};
    SamplerState samplerState_{};
  };
}