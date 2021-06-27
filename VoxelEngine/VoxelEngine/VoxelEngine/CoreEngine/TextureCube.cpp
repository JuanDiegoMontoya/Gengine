#include "PCH.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/TextureCube.h>
#include <stb_image.h>
#include <string>
#include <iostream>
#include <filesystem>

namespace GFX
{
  TextureCube::TextureCube(std::span<const std::string, 6> faces)
  {
    //stbi_set_flip_vertically_on_load(true);

    //glCreateTextures(GL_TEXTURE_3D, 1, &rendererID_);
    //int i = 0;
    //for (auto path : paths)
    //{
    //  std::string tex = texPath + path;
    //  bool hasTex = std::filesystem::exists(texPath + std::string(path));
    //  if (hasTex == false)
    //  {
    //    std::cout << "Failed to load texture " << path << ", using fallback.\n";
    //    tex = texPath + "error.png";
    //  }

    //  int n, dimX, dimY;
    //  auto pixels = (unsigned char*)stbi_load(tex.c_str(), &dimX, &dimY, &n, 4);
    //  ASSERT(pixels != nullptr);

    //  // TODO: play with this parameter for optimal looks, maybe make it user-selectable
    //  glTextureParameteri(rendererID_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //  glTextureParameteri(rendererID_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //  glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //  glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //  glTextureParameteri(rendererID_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //  glTextureStorage3D(rendererID_, 1, GL_RGBA, dimX, dimY, 6);
    //  glTextureSubImage3D(
    //    rendererID_,
    //    0,              // mip level 0
    //    0, 0, i,        // image start layer
    //    dimX, dimY, 1,  // x, y, z size
    //    GL_RGB,
    //    GL_UNSIGNED_BYTE,
    //    pixels);

    //  stbi_image_free(pixels);
    //  i++;
    //}

    stbi_set_flip_vertically_on_load(false);
    glGenTextures(1, &rendererID_);
    glBindTexture(GL_TEXTURE_CUBE_MAP, rendererID_);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
      std::string tex = texPath + faces[i];
      bool hasTex = std::filesystem::exists(texPath + faces[i]);
      if (hasTex == false)
      {
        std::cout << "Failed to load texture " << faces[i] << ", using fallback.\n";
        tex = texPath + "error.png";
      }
      //unsigned char* data = stbi_load(tex.c_str(), &width, &height, &nrChannels, 3);
      float* data = stbi_loadf(tex.c_str(), &width, &height, &nrChannels, 3);
      if (data)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
          0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
      }
      else
      {
        std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
      }
      stbi_image_free(data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }

  TextureCube& TextureCube::operator=(TextureCube&& rhs) noexcept
  {
    if (&rhs == this) return *this;
    return *new (this) TextureCube(std::move(rhs));
  }

  TextureCube::TextureCube(TextureCube&& rhs) noexcept
  {
    this->rendererID_ = std::exchange(rhs.rendererID_, 0);
  }

  TextureCube::~TextureCube()
  {
    glDeleteTextures(1, &rendererID_);
  }

  void TextureCube::Bind(unsigned slot) const
  {
    glBindTextureUnit(slot, rendererID_);
  }
}