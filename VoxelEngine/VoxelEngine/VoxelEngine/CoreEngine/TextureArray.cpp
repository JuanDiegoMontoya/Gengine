#include "EnginePCH.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/TextureArray.h>
#include <stb_image.h>
#include <filesystem>
#include <iostream>

namespace GFX
{
  TextureArray::TextureArray(std::span<std::string> textures, glm::ivec2 xyDim)
    : dim(xyDim)
  {
    const GLsizei layerCount = static_cast<GLsizei>(textures.size());
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &rendererID_);
    glTextureStorage3D(rendererID_, 1, GL_SRGB8_ALPHA8, dim.x, dim.y, layerCount);
    
    stbi_set_flip_vertically_on_load(true);

    int i = 0;
    for (auto texture : textures)
    {
      std::string tex = texPath + texture;
      bool hasTex = std::filesystem::exists(texPath + texture);

      if (hasTex == false)
      {
        std::cout << "Failed to load texture " << texture << ", using fallback.\n";
        tex = texPath + "error.png";
      }

      int width, height, n;
      auto pixels = (unsigned char*)stbi_load(tex.c_str(), &width, &height, &n, 4);
      ASSERT(pixels != nullptr);
      ASSERT(width == dim.x && height == dim.y);

      glTextureSubImage3D(
        rendererID_,
        0,           // mip level 0
        0, 0, i,     // image start layer
        dim.x, dim.y, 1, // x, y, z size (z = 1 b/c it's just a single layer)
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels);

      stbi_image_free(pixels);

      i++;
    }

    // sets the anisotropic filtering texture paramter to the highest supported by the system
    // TODO: make this parameter user-selectable
    GLfloat a;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &a);
    glTextureParameterf(rendererID_, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

    // TODO: play with this parameter for optimal looks, maybe make it user-selectable
    glTextureParameteri(rendererID_, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameteri(rendererID_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // use OpenGL to generate mipmaps for us
    glGenerateTextureMipmap(rendererID_);
  }


  TextureArray::~TextureArray()
  {
    glDeleteTextures(1, &rendererID_);
  }


  void TextureArray::Bind(unsigned slot) const
  {
    glBindTextureUnit(slot, rendererID_);
  }
}