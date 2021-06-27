#pragma once
#include <cinttypes>

namespace GFX
{
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
    R12G12B12A12,
    R16G16B16A16,
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

  enum class SampleCount
  {
    ONE,
    TWO,
    FOUR,
    EIGHT,
    SIXTEEN,
  };

  struct TextureCreateInfo
  {
    ImageType imageType = ImageType::TEX_2D;
    Format format = Format::UNDEFINED;
    Extent3D extent{};
    uint32_t mipLevels{};
    uint32_t arrayLayers{};
    SampleCount sampleCount = SampleCount::ONE;
  };

  //struct TextureUpdateInfo
  //{

  //};

  // serves as the raw storage for textures
  // cannot be used directly for samplers
  class Texture
  {
  public:
    Texture(const TextureCreateInfo& createInfo);
    Texture(Texture&& old) noexcept;
    Texture& operator=(Texture&& old) noexcept;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

  private:
    uint32_t id_{};
    TextureCreateInfo createInfo_{};
  };
}