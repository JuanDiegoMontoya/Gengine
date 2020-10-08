#include <Graphics/GraphicsIncludes.h>
#include "texture.h"
#include <stb_image.h>
#include <string>
#include <iostream>

const char* Texture::texture_dir_ = "./resources/Textures/";

Texture::Texture(const std::string & path)
  : rendererID_(0), filepath_(path), localbuffer_(nullptr), width_(0), height_(0), BPP_(0)
{
  std::string realpath = texture_dir_ + path;

  // top = 0
  stbi_set_flip_vertically_on_load(1);

  // 4 = rgba
  localbuffer_ = stbi_load(realpath.c_str(), &width_, &height_, &BPP_, 4);

  if (!localbuffer_)
    std::cout << "Failed to load texture: " << path << std::endl;

  glGenTextures(1, &rendererID_);
  glBindTexture(GL_TEXTURE_2D, rendererID_);
  
  // these four parameters must be specified by us
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glm::vec4 bordercolor(1, 0, 0, 1);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &bordercolor[0]);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // x
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); // y
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); // x
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); // y
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, localbuffer_);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (localbuffer_)
    stbi_image_free(localbuffer_);
}

Texture::~Texture()
{
  glDeleteTextures(1, &rendererID_);
}

void Texture::Bind(GLuint slot) const
{
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, rendererID_);
}

void Texture::Unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}
