#include "PCH.h"
#include "Texture.h"
#include <GL/glew.h>
#include <utility>

namespace GFX
{
  namespace
  {
    static GLint targets[]
    {
      GL_TEXTURE_1D,
      GL_TEXTURE_2D,
      GL_TEXTURE_3D,
      GL_TEXTURE_1D_ARRAY,
      GL_TEXTURE_2D_ARRAY,
      GL_TEXTURE_CUBE_MAP,
      //GL_TEXTURE_CUBE_MAP_ARRAY,
      GL_TEXTURE_2D_MULTISAMPLE,
      GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    };

    static GLint formats[]
    {
      0,
      GL_R8,
      GL_R8_SNORM,
      GL_R16,
      GL_R16_SNORM,
      GL_RG8,
      GL_RG8_SNORM,
      GL_RG16,
      GL_RG16_SNORM,
      GL_R3_G3_B2,
      GL_RGB4,
      GL_RGB5,
      GL_RGB8,
      GL_RGB8_SNORM,
      GL_RGB10,
      GL_RGB12,
      GL_RGB16_SNORM,
      GL_RGBA2,
      GL_RGBA4,
      GL_RGB5_A1,
      GL_RGBA8,
      GL_RGBA8_SNORM,
      GL_RGB10_A2,
      GL_RGB10_A2UI,
      GL_RGBA12,
      GL_RGBA16,
      GL_SRGB8,
      GL_SRGB8_ALPHA8,
      GL_R16F,
      GL_RG16F,
      GL_RGB16F,
      GL_RGBA16F,
      GL_R32F,
      GL_RG32F,
      GL_RGB32F,
      GL_RGBA32F,
      GL_R11F_G11F_B10F,
      GL_RGB9_E5,
      GL_R8I,
      GL_R8UI,
      GL_R16I,
      GL_R16UI,
      GL_R32I,
      GL_R32UI,
      GL_RG8I,
      GL_RG8UI,
      GL_RG16I,
      GL_RG16UI,
      GL_RG32I,
      GL_RG32UI,
      GL_RGB8I,
      GL_RGB8UI,
      GL_RGB16I,
      GL_RGB16UI,
      GL_RGB32I,
      GL_RGB32UI,
      GL_RGBA8I,
      GL_RGBA8UI,
      GL_RGBA16I,
      GL_RGBA16UI,
      GL_RGBA32I,
      GL_RGBA32UI,
    };

    static GLint sampleCounts[]{ 1, 2, 4, 8, 16 };
  }

  Texture::Texture(const TextureCreateInfo& createInfo)
    : createInfo_(createInfo)
  {
    glCreateTextures(targets[(int)createInfo.imageType], 1, &id_);

    switch (createInfo.imageType)
    {
    case ImageType::TEX_1D:
      glTextureStorage1D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width);
      break;
    case ImageType::TEX_2D:
      glTextureStorage2D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height);
      break;
    case ImageType::TEX_3D:
      glTextureStorage3D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height, createInfo.extent.depth);
      break;
    case ImageType::TEX_1D_ARRAY:
      glTextureStorage2D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width, createInfo.arrayLayers);
      break;
    case ImageType::TEX_2D_ARRAY:
      glTextureStorage3D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height, createInfo.arrayLayers);
      break;
    case ImageType::TEX_CUBEMAP:
      glTextureStorage2D(id_, createInfo.mipLevels, formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height);
      break;
    //case ImageType::TEX_CUBEMAP_ARRAY:
    //  ASSERT(false);
    //  break;
    case ImageType::TEX_2D_MULTISAMPLE:
      glTextureStorage2DMultisample(id_, sampleCounts[(int)createInfo.sampleCount], formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height, GL_FALSE);
      break;
    case ImageType::TEX_2D_MULTISAMPLE_ARRAY:
      glTextureStorage3DMultisample(id_, sampleCounts[(int)createInfo.sampleCount], formats[(int)createInfo.format], createInfo.extent.width, createInfo.extent.height, createInfo.arrayLayers, GL_FALSE);
      break;
    default:
      break;
    }
  }

  Texture::Texture(Texture&& old) noexcept
  {
    *this = std::move(old);
  }

  Texture& Texture::operator=(Texture&& old) noexcept
  {
    if (&old == this) return *this;
    id_ = std::exchange(old.id_, 0);
    createInfo_ = old.createInfo_;
    return *this;
  }

  Texture::~Texture()
  {
    glDeleteTextures(1, &id_);
  }
}