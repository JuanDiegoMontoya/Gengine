#pragma once
#include <cstdint>
#include <utility/Flags.h>

namespace GFX
{
  struct Extent3D
  {
    uint32_t width{};
    uint32_t height{};
    uint32_t depth{};

    bool operator==(const Extent3D&) const = default;
  };

  struct Extent2D
  {
    uint32_t width{};
    uint32_t height{};

    bool operator==(const Extent2D&) const = default;
  };

  struct Offset3D
  {
    uint32_t x{};
    uint32_t y{};
    uint32_t z{};

    bool operator==(const Offset3D&) const = default;
  };

  struct Offset2D
  {
    uint32_t x{};
    uint32_t y{};

    bool operator==(const Offset2D&) const = default;
  };

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
    R11G11B10_FLOAT,
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

    D32_FLOAT,
    D32_UNORM,
    D24_UNORM,
    D16_UNORM,
    D32_FLOAT_S8_UINT,
    D24_UNORM_S8_UINT,
    // TODO: compressed formats
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

  enum class Attachment
  {
    NONE,
    COLOR_0,
    COLOR_1,
    COLOR_2,
    COLOR_3,

    DEPTH,
    STENCIL,
    DEPTH_STENCIL,
  };

  enum class AspectMaskBit
  {
    COLOR_BUFFER_BIT    = 1 << 0,
    DEPTH_BUFFER_BIT    = 1 << 1,
    STENCIL_BUFFER_BIT  = 1 << 2,
  };
  DECLARE_FLAG_TYPE(AspectMaskBits, AspectMaskBit, uint32_t)
}