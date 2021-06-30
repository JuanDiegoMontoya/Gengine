#pragma once
#include "Texture.h"

namespace GFX
{
  std::optional<Texture> LoadTexture2D(std::string_view file,
    Format internalFormat = Format::R8G8B8A8_SRGB,
    UploadFormat uploadFormat = UploadFormat::RGBA,
    UploadType uploadType = UploadType::UBYTE);
}