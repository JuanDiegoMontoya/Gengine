#pragma once
#include "Texture.h"

namespace GFX
{
  // loads a 2D texture from a file
  // has sane defaults, optimized for basic texture mapping
  [[nodiscard]] std::optional<Texture> LoadTexture2D(const std::string_view file,
    Format internalFormat = Format::R8G8B8A8_SRGB,
    UploadType uploadType = UploadType::UBYTE);

  [[nodiscard]] std::optional<Texture> LoadTexture2DArray(std::span<const std::string_view> files,
    uint32_t xSize = 0, uint32_t ySize = 0,
    Format internalFormat = Format::R8G8B8A8_SRGB,
    UploadType uploadType = UploadType::UBYTE);

  [[nodiscard]] std::optional<Texture> LoadTextureCube(std::span<const std::string_view, 6> files,
    uint32_t xSize = 0, uint32_t ySize = 0,
    Format internalFormat = Format::R16G16B16_FLOAT,
    UploadType uploadType = UploadType::FLOAT);
}