#pragma once
#include <unordered_map>
#include <span>
#include <Utilities/HashedString.h>

namespace GFX
{
  // simple shader interface
  class Shader
  {
  public:
    void Bind() const;

    void SetBool(hashed_string uniform, bool value);
    void SetInt(hashed_string uniform, int value);
    void SetUInt(hashed_string uniform, unsigned int value);
    void SetFloat(hashed_string uniform, float value);
    void Set1FloatArray(hashed_string uniform, std::span<const float> value);
    void Set2FloatArray(hashed_string uniform, std::span<const glm::vec2> value);
    void Set3FloatArray(hashed_string uniform, std::span<const glm::vec3> value);
    void Set4FloatArray(hashed_string uniform, std::span<const glm::vec4> value);
    void SetIntArray(hashed_string uniform, std::span<const int> value);
    void SetVec2(hashed_string uniform, const glm::vec2& value);
    void SetIVec2(hashed_string uniform, const glm::ivec2& value);
    void SetVec3(hashed_string uniform, const glm::vec3& value);
    void SetVec4(hashed_string uniform, const glm::vec4& value);
    void SetMat3(hashed_string uniform, const glm::mat3& mat);
    void SetMat4(hashed_string uniform, const glm::mat4& mat);

    Shader(const Shader&) = default;
    Shader(Shader&&) = default;
    Shader& operator=(const Shader&) = default;
    Shader& operator=(Shader&&) = default;

  private:
    friend class ShaderManager;
    Shader(std::unordered_map<uint32_t, uint32_t>& uniforms, uint32_t id)
      : uniformIDs_(uniforms), id_(id)
    {
    }

    std::unordered_map<uint32_t, uint32_t>& uniformIDs_;
    uint32_t id_{};
  };
}