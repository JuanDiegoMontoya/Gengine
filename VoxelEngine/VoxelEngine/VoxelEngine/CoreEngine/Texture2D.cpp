#include "PCH.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/Texture2D.h>
#include <stb_image.h>
#include <string>
#include <iostream>
#include <filesystem>

namespace GFX
{
  Texture2D::Texture2D(std::string_view path)
  {
    stbi_set_flip_vertically_on_load(true);

    std::string tex = texPath + std::string(path);
    bool hasTex = std::filesystem::exists(texPath + std::string(path));
    if (hasTex == false)
    {
      std::cout << "Failed to load texture " << path << ", using fallback.\n";
      tex = texPath + "error.png";
    }

    int n;
    auto pixels = (unsigned char*)stbi_load(tex.c_str(), &dim_.x, &dim_.y, &n, 4);
    ASSERT(pixels != nullptr);

    glCreateTextures(GL_TEXTURE_2D, 1, &rendererID_);

    // sets the anisotropic filtering texture paramter to the highest supported by the system
    // TODO: make this parameter user-selectable
    GLfloat a;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &a);
    glTextureParameterf(rendererID_, GL_TEXTURE_MAX_ANISOTROPY, a);

    // TODO: play with this parameter for optimal looks, maybe make it user-selectable
    glTextureParameteri(rendererID_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(rendererID_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTextureStorage2D(rendererID_, 1, GL_SRGB8_ALPHA8, dim_.x, dim_.y);
    glTextureSubImage2D(
      rendererID_,
      0,              // mip level 0
      0, 0,           // image start layer
      dim_.x, dim_.y, // x, y, z size (z = 1 b/c it's just a single layer)
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      pixels);

    stbi_image_free(pixels);

    // use OpenGL to generate mipmaps for us
    glGenerateTextureMipmap(rendererID_);
  }

  Texture2D& Texture2D::operator=(Texture2D&& rhs) noexcept
  {
    if (&rhs == this) return *this;
    return *new (this) Texture2D(std::move(rhs));
  }

  Texture2D::Texture2D(Texture2D&& rhs) noexcept
  {
    this->rendererID_ = std::exchange(rhs.rendererID_, 0);
    this->dim_ = rhs.dim_;
  }

  Texture2D::~Texture2D()
  {
    glDeleteTextures(1, &rendererID_);
  }

  void Texture2D::Bind(unsigned slot) const
  {
    glBindTextureUnit(slot, rendererID_);
  }
}