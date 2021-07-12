#pragma once
#include <utility/HashedString.h>
#include <optional>
#include "Shader.h"

namespace GFX
{
  enum class ShaderType
  {
    UNDEFINED,
    VERTEX,
    TESS_CONTROL,
    TESS_EVAL,
    GEOMETRY,
    FRAGMENT,
    COMPUTE,
  };

  struct ShaderCreateInfo
  {
    std::string path{};
    ShaderType type{};
    std::vector<std::pair<std::string, std::string>> replace{};
  };

  class ShaderManager
  {
  public:
    [[nodiscard]] static ShaderManager* Get();
    std::optional<Shader> AddShader(hashed_string name, const std::vector<ShaderCreateInfo>& createInfos);
    [[nodiscard]] std::optional<Shader> GetShader(hashed_string name);

  private:
    ShaderManager();
    ~ShaderManager();

    struct ShaderManagerStorage* storage;
  };
}