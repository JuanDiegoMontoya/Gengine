#include "PCH.h"
#include "TextureLoader.h"
#include <stb_image.h>
#include <filesystem>
#include <glad/glad.h>

namespace GFX
{

  namespace
  {
    enum class TypeWidth : uint8_t
    {
      BYTE,
      SHORT,
      WORD,
    };

    TypeWidth GetUploadTypeWidth(UploadType type)
    {
      ASSERT(type != UploadType::UNDEFINED);

      switch (type)
      {
      case UploadType::UBYTE:
      case UploadType::SBYTE:
      case UploadType::UBYTE_3_3_2:
      case UploadType::UBYTE_2_3_3:
        return TypeWidth::BYTE;
      case UploadType::USHORT:
      case UploadType::SSHORT:
      case UploadType::USHORT_5_6_5:
      case UploadType::USHORT_5_6_5_REV:
      case UploadType::USHORT_4_4_4_4:
      case UploadType::USHORT_4_4_4_4_REV:
      case UploadType::USHORT_5_5_5_1:
      case UploadType::USHORT_5_5_5_1_REV:
        return TypeWidth::SHORT;
      case UploadType::UINT:
      case UploadType::SINT:
      case UploadType::FLOAT:
      case UploadType::UINT_8_8_8_8:
      case UploadType::UINT_8_8_8_8_REV:
      case UploadType::UINT_10_10_10_2:
      case UploadType::UINT_10_10_10_2_REV:
        return TypeWidth::WORD;
      default:
        break;
      }

      ASSERT(0);
      return TypeWidth::WORD;
      // UNREACHABLE
    }

    struct TextureData
    {
      int x{};
      int y{};
      void* pixels{};
      TypeWidth width{};
    };

    TextureData LoadTextureBase(const char* file, UploadType uploadFormat)
    {
      TextureData data;
      switch (GetUploadTypeWidth(uploadFormat))
      {
      case TypeWidth::BYTE:
        data.pixels = stbi_load(file, &data.x, &data.y, nullptr, 4);
        break;
      case TypeWidth::SHORT:
      case TypeWidth::WORD:
      default:
        stbi_ldr_to_hdr_gamma(1.0f);
        data.pixels = stbi_loadf(file, &data.x, &data.y, nullptr, 4);
        stbi_ldr_to_hdr_gamma(2.2f);
      }

      return data;
    }
  }


  std::optional<Texture> LoadTexture2D(const std::string_view file,
    Format internalFormat,
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
    auto texture = Texture::Create(createInfo, file);

    TextureUpdateInfo updateInfo
    {
      .dimension = UploadDimension::TWO,
      .level = 0,
      .offset{ 0, 0, 0 },
      .size = createInfo.extent,
      .format = UploadFormat::RGBA,
      .type = uploadType,
      .pixels = pixels
    };
    texture->SubImage(updateInfo);

    texture->GenMipmaps();

    stbi_image_free(pixels);

    return texture;
  }


  std::optional<Texture> LoadTexture2DArray(std::span<const std::string_view> files,
    uint32_t xSize, uint32_t ySize,
    Format internalFormat,
    UploadType uploadType)
  {
    if (files.empty())
    {
      return std::nullopt;
    }

    bool infer_x_size = xSize == 0;
    bool infer_y_size = ySize == 0;

    stbi_set_flip_vertically_on_load(true);

    stbi_ldr_to_hdr_gamma(1.0f);
    std::vector<float*> texturesCPU;
    for (const auto& file : files)
    {
      std::string texPath = std::string(TextureDir) + std::string(file);
      bool hasTex = std::filesystem::exists(texPath);
      if (hasTex == false)
      {
        for (auto pixels : texturesCPU) stbi_image_free(pixels);
        return std::nullopt;
      }

      int xDim{};
      int yDim{};
      auto* pixels = (float*)stbi_loadf(texPath.c_str(), &xDim, &yDim, nullptr, 4);
      if (pixels == nullptr || (!infer_x_size && xDim != xSize) || (!infer_y_size && yDim != ySize))
      {
        for (auto pixels : texturesCPU) stbi_image_free(pixels);
        return std::nullopt;
      }
      if (infer_x_size) xSize = static_cast<uint32_t>(xDim);
      if (infer_y_size) ySize = static_cast<uint32_t>(yDim);
      texturesCPU.push_back(pixels);
    }
    stbi_ldr_to_hdr_gamma(2.2f);

    uint32_t numMips = 1 + std::floor(std::log2((float)std::max(xSize, ySize)));

    TextureCreateInfo createInfo
    {
      .imageType = ImageType::TEX_2D_ARRAY,
      .format = internalFormat,
      .extent
      {
        .width = xSize,
        .height = ySize,
        .depth = 1
      },
      .mipLevels = numMips,
      .arrayLayers = static_cast<uint32_t>(files.size()),
      .sampleCount = SampleCount::ONE,
    };
    auto texture = Texture::Create(createInfo, files[0]);

    for (uint32_t i = 0; auto pixels : texturesCPU)
    {
      TextureUpdateInfo updateInfo
      {
        .dimension = UploadDimension::THREE,
        .level = 0,
        .offset{ 0, 0, i },
        .size = createInfo.extent,
        .format = UploadFormat::RGBA,
        .type = UploadType::FLOAT,
        .pixels = pixels
      };
      texture->SubImage(updateInfo);
      stbi_image_free(pixels);
      i++;
    }

    texture->GenMipmaps();

    return texture;
  }


  std::optional<Texture> LoadTextureCube(std::span<const std::string_view, 6> files,
    uint32_t xSize, uint32_t ySize,
    Format internalFormat,
    UploadType uploadType)
  {
    bool infer_x_size = xSize == 0;
    bool infer_y_size = ySize == 0;

    stbi_set_flip_vertically_on_load(false);

    std::vector<float*> texturesCPU;
    for (const auto& file : files)
    {
      std::string texPath = std::string(TextureDir) + std::string(file);
      bool hasTex = std::filesystem::exists(texPath);
      if (hasTex == false)
      {
        for (auto pixels : texturesCPU) stbi_image_free(pixels);
        return std::nullopt;
      }

      int xDim{};
      int yDim{};
      float* pixels = (float*)stbi_loadf(texPath.c_str(), &xDim, &yDim, nullptr, 4);
      if (pixels == nullptr || (!infer_x_size && xDim != xSize) || (!infer_y_size && yDim != ySize))
      {
        for (auto pixels : texturesCPU) stbi_image_free(pixels);
        return std::nullopt;
      }
      if (infer_x_size) xSize = static_cast<uint32_t>(xDim);
      if (infer_y_size) ySize = static_cast<uint32_t>(yDim);
      texturesCPU.push_back(pixels);
    }

    TextureCreateInfo createInfo
    {
      .imageType = ImageType::TEX_CUBEMAP,
      .format = internalFormat,
      .extent
      {
        .width = xSize,
        .height = ySize,
        .depth = 1
      },
      .mipLevels = 1,
      .arrayLayers = 6,
      .sampleCount = SampleCount::ONE,
    };
    auto texture = Texture::Create(createInfo, files[0]);

    for (uint32_t i = 0; auto pixels : texturesCPU)
    {
      TextureUpdateInfo updateInfo
      {
        .dimension = UploadDimension::THREE,
        .level = 0,
        .offset{ 0, 0, i },
        .size = createInfo.extent,
        .format = UploadFormat::RGBA,
        .type = uploadType,
        .pixels = pixels
      };
      texture->SubImage(updateInfo);
      stbi_image_free(pixels);
      i++;
    }

    return texture;
  }
}