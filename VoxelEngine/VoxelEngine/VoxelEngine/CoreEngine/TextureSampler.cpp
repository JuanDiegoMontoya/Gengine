#include "PCH.h"
#include "TextureSampler.h"
#include <GL/glew.h>
#include <utility>

template <typename H, typename T, typename... Rest>
static void hashCombine(H& seed, const T& v, Rest... rest)
{
  seed ^= std::hash<H>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
}

namespace GFX
{
  namespace
  {
    static GLint addressModes[]
    {
      GL_REPEAT,
      GL_MIRRORED_REPEAT,
      GL_CLAMP_TO_EDGE,
      GL_CLAMP_TO_BORDER,
      GL_MIRROR_CLAMP_TO_EDGE,
    };

    static GLfloat anisotropies[]{ 1, 2, 4, 8, 16 };
  }

  TextureSampler::TextureSampler(const SamplerState& state)
  {
    glCreateSamplers(1, &id_);
    SetState(state, true);
  }

  TextureSampler::TextureSampler(TextureSampler&& old) noexcept
  {
    *this = std::move(old);
  }

  TextureSampler& TextureSampler::operator=(TextureSampler&& old) noexcept
  {
    if (&old == this) return *this;
    id_ = std::exchange(old.id_, 0);
    samplerState_ = old.samplerState_;
    return *this;
  }

  TextureSampler::~TextureSampler()
  {
    glDeleteSamplers(1, &id_);
  }

  void TextureSampler::SetState(const SamplerState& state)
  {
    SetState(state, false);
  }

  void TextureSampler::SetState(const SamplerState& state, bool force)
  {
    // early out
    if (state.asUint32 == samplerState_.asUint32)
      return;

    if (state.asBitField.magFilter != samplerState_.asBitField.magFilter)
    {
      GLint filter = state.asBitField.magFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST;
      glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, filter);
    }
    if (state.asBitField.minFilter != samplerState_.asBitField.minFilter)
    {
      GLint filter{};
      if (state.asBitField.minFilter == Filter::LINEAR)
        filter = state.asBitField.mipmapFilter == Filter::LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
      else
        filter = state.asBitField.mipmapFilter == Filter::LINEAR ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
      glSamplerParameteri(id_, GL_TEXTURE_MAG_FILTER, filter);
    }

    if (state.asBitField.addressModeU != samplerState_.asBitField.addressModeU || force)
      glSamplerParameteri(id_, GL_TEXTURE_WRAP_S, addressModes[(int)state.asBitField.addressModeU]);
    if (state.asBitField.addressModeV != samplerState_.asBitField.addressModeV || force)
      glSamplerParameteri(id_, GL_TEXTURE_WRAP_T, addressModes[(int)state.asBitField.addressModeV]);
    if (state.asBitField.addressModeW != samplerState_.asBitField.addressModeW || force)
      glSamplerParameteri(id_, GL_TEXTURE_WRAP_R, addressModes[(int)state.asBitField.addressModeW]);

    if (state.asBitField.anisotropy != samplerState_.asBitField.anisotropy)
      glSamplerParameterf(id_, GL_TEXTURE_MAX_ANISOTROPY, anisotropies[(int)state.asBitField.anisotropy]);
  }
}