#pragma once
#include "utilities.h"
#include <unordered_map>
#include <optional>
#include <span>
#include <CoreEngine/GAssert.h>

#include <CoreEngine/GraphicsIncludes.h>
#include <shaderc/shaderc.hpp>
#include <entt/src/core/hashed_string.hpp>

#pragma warning(push)
#pragma warning(disable : 4267) // 8->4 byte int conversion

namespace std
{
  template<>
  struct hash<entt::hashed_string>
  {
    std::size_t operator()(const entt::hashed_string& hs) const
    {
      return hs.value();
    }
  };
}

// encapsulates shaders by storing uniforms and its GPU memory location
// also stores the program's name and both shader paths for recompiling
class Shader
{
public:
  using glShaderType = GLint;

  // standard constructor
  Shader(
    std::optional<std::string> vertexPath,
    std::optional<std::string> fragmentPath,
    std::optional<std::string> tessCtrlPath = std::nullopt,
    std::optional<std::string> tessEvalPath = std::nullopt,
    std::optional<std::string> geometryPath = std::nullopt);

  // compute shader constructor
  Shader(int, std::string computePath);

  // universal SPIR-V constructor (takes a list of paths and shader types)
  Shader(std::vector<std::pair<std::string, glShaderType>> shaders);

  // default constructor (currently no uses)
  Shader()
  {
    //type = sDefault;
    programID = 0;
  }

  // move constructor
  Shader(Shader&& other) noexcept : Uniforms(std::move(other.Uniforms)), programID(other.programID)
  {
    other.programID = 0;
  }

  ~Shader()
  {
    if (programID != 0)
      glDeleteProgram(programID);
  }

  // set the active shader to this one
  void Use() const
  {
    glUseProgram(programID);
  }

  void Unuse() const
  {
    glUseProgram(0);
  }

  void setBool(entt::hashed_string uniform, bool value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1i(programID, Uniforms[uniform], static_cast<int>(value));
  }
  void setInt(entt::hashed_string uniform, int value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1i(programID, Uniforms[uniform], value);
  }
  void setUInt(entt::hashed_string uniform, int value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1ui(programID, Uniforms[uniform], value);
  }
  void setFloat(entt::hashed_string uniform, float value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1f(programID, Uniforms[uniform], value);
  }
  void set1FloatArray(entt::hashed_string uniform, std::span<const float> value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1fv(programID, Uniforms[uniform], value.size(), value.data());
  }
  void set1FloatArray(entt::hashed_string uniform, const float* value, GLsizei count)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1fv(programID, Uniforms[uniform], count, value);
  }
  void set2FloatArray(entt::hashed_string uniform, std::span<const glm::vec2> value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform2fv(programID, Uniforms[uniform], value.size(), glm::value_ptr(value.front()));
  }
  void set3FloatArray(entt::hashed_string uniform, std::span<const glm::vec3> value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform3fv(programID, Uniforms[uniform], value.size(), glm::value_ptr(value.front()));
  }
  void set4FloatArray(entt::hashed_string uniform, std::span<const glm::vec4> value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform4fv(programID, Uniforms[uniform], value.size(), glm::value_ptr(value.front()));
  }
  void setIntArray(entt::hashed_string uniform, std::span<const int> value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform1iv(programID, Uniforms[uniform], value.size(), value.data());
  }
  void setVec2(entt::hashed_string uniform, const glm::vec2& value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform2fv(programID, Uniforms[uniform], 1, glm::value_ptr(value));
  }
  void setVec2(entt::hashed_string uniform, float x, float y)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform2f(programID, Uniforms[uniform], x, y);
  }
  void setVec3(entt::hashed_string uniform, const glm::vec3& value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform3fv(programID, Uniforms[uniform], 1, glm::value_ptr(value));
  }
  void setVec3(entt::hashed_string uniform, float x, float y, float z)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform3f(programID, Uniforms[uniform], x, y, z);
  }
  void setVec4(entt::hashed_string uniform, const glm::vec4 &value)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform4fv(programID, Uniforms[uniform], 1, glm::value_ptr(value));
  }
  void setVec4(entt::hashed_string uniform, float x, float y, float z, float w)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniform4f(programID, Uniforms[uniform], x, y, z, w);
  }
  void setMat3(entt::hashed_string uniform, const glm::mat3 &mat)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniformMatrix3fv(programID, Uniforms[uniform], 1, GL_FALSE, glm::value_ptr(mat));
  }
  void setMat4(entt::hashed_string uniform, const glm::mat4& mat)
  {
    ASSERT(Uniforms.find(uniform) != Uniforms.end());
    glProgramUniformMatrix4fv(programID, Uniforms[uniform], 1, GL_FALSE, glm::value_ptr(mat));
  }

  // list of all shader programs
  static inline std::unordered_map<entt::hashed_string, 
    std::optional<Shader>> shaders;
private:

  std::unordered_map<entt::hashed_string, GLint> Uniforms;
  GLuint programID{ 0 };

  using shaderType = GLenum;
  friend class IncludeHandler;

  // shader dir includes source and headers alike
  static constexpr const char* shader_dir_ = "./Resources/Shaders/";
  static std::string loadFile(std::string path);

  GLint compileShader(shaderType type, const std::vector<std::string>& src, std::string_view path);
  void initUniforms();
  void checkLinkStatus(std::vector<std::string_view> files);

  // returns compiled SPIR-V
  std::vector<uint32_t>
    spvPreprocessAndCompile(
      shaderc::Compiler& compiler,
      const shaderc::CompileOptions options,
      std::string path,
      shaderc_shader_kind a);

};

#pragma warning(pop)