#include "PCH.h"
#include "TextureLoader.h"
#include <stb_image.h>
#include <filesystem>
#include <GL/glew.h>

namespace GFX
{
  std::optional<Texture> LoadTexture2D(std::string_view file,
    Format internalFormat,
    UploadFormat uploadFormat,
    UploadType uploadType)
  {
    stbi_set_flip_vertically_on_load(true);

    std::string texPath = std::string(TextureDir) + std::string(file);
    bool hasTex = std::filesystem::exists(texPath);
    if (hasTex == false)
    {
      return std::nullopt;
    }

    int xDim{};
    int yDim{};
    auto pixels = (unsigned char*)stbi_load(texPath.c_str(), &xDim, &yDim, nullptr, 4);
    if (pixels == nullptr)
    {
      return std::nullopt;
    }

    uint32_t numMips = 1 + std::floor(std::log2((float)std::max(xDim, yDim)));

    TextureCreateInfo createInfo
    {
      .imageType = ImageType::TEX_2D,
      .format = internalFormat,
      .extent
      {
        .width = static_cast<uint32_t>(xDim),
        .height = static_cast<uint32_t>(yDim),
        .depth = 1
      },
      .mipLevels = numMips,
      .arrayLayers = 1,
      .sampleCount = SampleCount::ONE,
    };
    auto texture = Texture::Create(createInfo);

    TextureUpdateInfo updateInfo
    {
      .dimension = UploadDimension::TWO,
      .level = 0,
      .offset{ 0, 0, 0 },
      .size = createInfo.extent,
      .format = uploadFormat,
      .type = uploadType,
      .pixels = pixels
    };
    texture->SubImage(updateInfo);

    texture->GenMipmaps();

    stbi_image_free(pixels);

    return texture;
  }
}