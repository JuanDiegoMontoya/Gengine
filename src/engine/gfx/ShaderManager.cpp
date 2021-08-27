#include "../PCH.h"
#include <glad/glad.h>
#include <shaderc/shaderc.hpp>
#include "ShaderManager.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <regex>
#include <utility/Defer.h>

namespace GFX
{
  namespace
  {
    shaderc_shader_kind shaderTypeToShadercType[]
    {
      shaderc_glsl_infer_from_source,
      shaderc_vertex_shader,
      shaderc_tess_control_shader,
      shaderc_tess_evaluation_shader,
      shaderc_geometry_shader,
      shaderc_fragment_shader,
      shaderc_compute_shader,
    };

    GLenum shaderTypeToGLType[]
    {
      0,
      GL_VERTEX_SHADER,
      GL_TESS_CONTROL_SHADER,
      GL_TESS_EVALUATION_SHADER,
      GL_GEOMETRY_SHADER,
      GL_FRAGMENT_SHADER,
      GL_COMPUTE_SHADER,
    };
  }

  struct ShaderData
  {
    std::string name;
    std::unordered_map<uint32_t, uint32_t> uniformIDs;
    uint32_t id{};
    std::vector<ShaderCreateInfo> createInfos;
  };

  static std::string LoadFile(std::string_view path)
  {
    std::string shaderpath = std::string(ShaderDir) + std::string(path);
    std::string content;
    try
    {
      std::ifstream ifs(shaderpath);
      content = std::string((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    }
    catch (std::ifstream::failure e)
    {
      std::cout << "Error reading shader file: " << path << '\n';
      std::cout << "Message: " << e.what() << std::endl;
    }
    return content;
  }

  GLuint CompileShader(GLenum type, const std::string& src, std::string_view path)
  {
    GLuint shader = 0;
    GLchar infoLog[512];
    GLint success;

    shader = glCreateShader(type);

    const GLchar* strings = src.c_str();

    glShaderSource(shader, 1, &strings, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);

      std::cout << "File: " << path << std::endl;
      std::cout << "Error compiling shader of type " << type << '\n' << infoLog << std::endl;
      return 0;
    }

    return shader;
  }

  //std::string HandleIncludes([[maybe_unused]] std::string_view path, std::string_view rawSource)
  //{
  //  std::stringstream in{ std::string(rawSource) };
  //  std::stringstream out;

  //  enum class CommentType { None, CStyle, CppStyle };
  //  CommentType inComment = CommentType::None;
  //  for (std::string line; std::getline(in, line, '\n');)
  //  {
  //    if (inComment == CommentType::None)
  //    {
  //      auto findRes = line.find("#include ", 0);
  //      if (findRes != std::string::npos)
  //      {
  //        auto includePath = line.substr(line.find_first_of, line.size() - 2);
  //        return includePath;
  //      }
  //    }
  //  }

  //  return out.str();
  //}

  std::string PreprocessShader(
    shaderc::Compiler& compiler,
    const shaderc::CompileOptions options,
    const std::vector<std::pair<std::string, std::string>>& replace,
    std::string path,
    shaderc_shader_kind shaderType)
  {
    std::string rawSrc = LoadFile(path);
    for (const auto& [search, replacement] : replace)
    {
      rawSrc = std::regex_replace(rawSrc, std::regex(search), replacement);
    }
    
    auto PreprocessResult = compiler.PreprocessGlsl(
      rawSrc, shaderType, path.c_str(), options);
    if (auto numErr = PreprocessResult.GetNumErrors(); numErr > 0)
    {
      PreprocessResult.GetCompilationStatus();
      spdlog::error("{} errors preprocessing {}!\n", numErr, path);
      spdlog::error("{}", PreprocessResult.GetErrorMessage());
      return {};
    }

    //if (auto str = HandleIncludes(path, rawSrc); !str.empty())
    //{
    //  spdlog::debug("Include: {}", str);
    //}

    std::string preprocessed = PreprocessResult.begin();
    // TODO: uncomment when extension capabilities can be queried effectively
    //preprocessed = std::regex_replace(preprocessed,
    //  std::regex("#((extension[\n\t ]+(GL_GOOGLE_include_directive))|(line[\n\t ]+\"))[a - zA - Z0 - 9\"_:. ]+"), "// line removed for combatibility");

    return preprocessed;
  }

  // returns compiled SPIR-V
  std::vector<uint32_t>
    PreprocessAndCompileSPIRV(
      shaderc::Compiler& compiler,
      const shaderc::CompileOptions options,
      const std::vector<std::pair<std::string, std::string>>& replace,
      std::string path,
      shaderc_shader_kind shaderType)
  {
    std::string preprocessResult = PreprocessShader(compiler, options, replace, path, shaderType);

    auto CompileResult = compiler.CompileGlslToSpv(
      preprocessResult.c_str(), shaderType, path.c_str(), options);
    if (auto numErr = CompileResult.GetNumErrors(); numErr > 0)
    {
      printf("%llu errors compiling %s!\n", numErr, path.c_str());
      printf("%s", CompileResult.GetErrorMessage().c_str());
      return {};
    }

    return { CompileResult.begin(), CompileResult.end() };
  }

  class IncludeHandler : public shaderc::CompileOptions::IncluderInterface
  {
  public:
    virtual shaderc_include_result* GetInclude(
      const char* requested_source,
      [[maybe_unused]] shaderc_include_type type,
      [[maybe_unused]] const char* requesting_source,
      [[maybe_unused]] size_t include_depth)
    {
      auto* data = new shaderc_include_result;

      content = LoadFile(requested_source);
      source_name = requested_source;

      data->content = content.c_str();
      data->source_name = source_name.c_str();
      data->content_length = content.size();
      data->source_name_length = source_name.size();
      data->user_data = nullptr;

      return data;
    }

    virtual void ReleaseInclude([[maybe_unused]] shaderc_include_result* data)
    {
    }

  private:
    std::string content{};
    std::string source_name{};
  };

  void InitUniforms(ShaderData& program)
  {
    GLint uniform_count = 0;
    glGetProgramiv(program.id, GL_ACTIVE_UNIFORMS, &uniform_count);

    if (uniform_count > 0)
    {
      GLint max_name_len = 0;
      glGetProgramiv(program.id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_len);

      //auto uniform_name = std::make_unique<char[]>(max_name_len);
      std::string uniform_name(max_name_len + 1, '\0');

      for (GLint i = 0; i < uniform_count; ++i)
      {
        GLsizei length = 0;
        GLsizei count = 0;
        GLenum 	type = GL_NONE;
        glGetActiveUniform(program.id, i, max_name_len, &length, &count, &type, uniform_name.data());

        GLuint uniform_info = {};
        uniform_info = glGetUniformLocation(program.id, uniform_name.c_str());

        program.uniformIDs.emplace(hashed_string(uniform_name.c_str()), uniform_info);
        //Uniforms.emplace(hashed_string(uniform_name.get()), uniform_info);
      }
    }
  }

  bool CheckLinkStatus(std::vector<std::string_view> files, GLuint programID)
  {
    // link program
    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
      GLsizei length;
      glGetProgramInfoLog(programID, 0, &length, nullptr);
      std::string infoLog(length + 1, '\0');
      glGetProgramInfoLog(programID, length, nullptr, infoLog.data());
      std::cout << "Failed to link shader program! File(s):\n";
      for (const auto& file : files)
        std::cout << file << (file == *(files.end() - 1) ? "" : ", "); // no comma on last element
      std::cout << '\n';
      std::cout << "Error message:\n" << infoLog << std::endl;
      return false;
    }
    return true;
  }

  ShaderData CompileShader(hashed_string name, const std::vector<ShaderCreateInfo>& createInfos)
  {
    ShaderData shader;
    shaderc::Compiler compiler;
    ASSERT(compiler.IsValid());

    shaderc::CompileOptions options;
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetTargetEnvironment(shaderc_target_env_opengl, 450);
    options.SetIncluder(std::make_unique<IncludeHandler>());
    options.SetWarningsAsErrors();
    options.SetAutoMapLocations(true);
    options.SetAutoBindUniforms(true);

    std::vector<GLuint> shaderIDs;
    Defer deleteShaders = [&shaderIDs]()
    {
      for (auto shaderID : shaderIDs)
      {
        glDeleteShader(shaderID);
      }
    };

    for (auto& [shaderPath, shaderType, replace] : createInfos)
    {
      std::string preprocessedSource = PreprocessShader(compiler, options, replace, shaderPath, shaderTypeToShadercType[(int)shaderType]);
      GLuint shaderID = CompileShader(shaderTypeToGLType[(int)shaderType], preprocessedSource, shaderPath);
      if (shaderID == 0)
      {
        return shader;
      }
      shaderIDs.push_back(shaderID);
    }

    GLuint programID = glCreateProgram();
    if (programID == 0)
    {
      return shader;
    }

    Defer detachShaders = [&shaderIDs, programID]()
    {
      for (auto shaderID : shaderIDs)
      {
        glDetachShader(programID, shaderID);
      }
    };

    for (auto ID : shaderIDs)
    {
      glAttachShader(programID, ID);
    }

    glLinkProgram(programID);

    std::vector<std::string_view> strs;
    for (const auto& [shaderPath, shaderType, replace] : createInfos)
    {
      strs.push_back(shaderPath);
    }

    if (!CheckLinkStatus(strs, programID))
    {
      return shader;
    }

    shader.name = name.data();
    shader.id = programID;
    shader.createInfos = createInfos;
    InitUniforms(shader);
    //printf("Shader: %s\n", name.data());
    glObjectLabel(GL_PROGRAM, shader.id, -1, name.data());

    //storage->shaders.emplace(name, std::move(shader));
    //auto& shr = storage->shaders[name];
    //return Shader(shr.uniformIDs, shr.id);
    return shader;
  }



  struct ShaderManagerStorage
  {
    std::unordered_map<uint32_t, ShaderData> shaders;
  };


  ShaderManager* ShaderManager::Get()
  {
    static ShaderManager manager;
    return &manager;
  }

  ShaderManager::ShaderManager()
  {
    storage = new ShaderManagerStorage;
  }

  ShaderManager::~ShaderManager()
  {
    delete storage;
  }

  std::optional<Shader> ShaderManager::AddShader(hashed_string name, const std::vector<ShaderCreateInfo>& createInfos)
  {
    auto data = CompileShader(name, createInfos);
    if (data.id != 0 && !storage->shaders.contains(name))
    {
      auto it = storage->shaders.emplace(name, std::move(data));
      return Shader(it.first->second.uniformIDs, it.first->second.id);
    }
    return std::nullopt;
  }

  std::optional<Shader> ShaderManager::GetShader(hashed_string name)
  {
    if (auto it = storage->shaders.find(name); it != storage->shaders.end())
      return Shader(it->second.uniformIDs, it->second.id);
    return std::nullopt;
  }

  std::optional<Shader> ShaderManager::RecompileShader(hashed_string name)
  {
    auto it = storage->shaders.find(name);
    if (it == storage->shaders.end())
    {
      return std::nullopt;
    }

    auto data = CompileShader(name, it->second.createInfos);
    if (data.id == 0) // compile failed
    {
      return std::nullopt;
    }

    glDeleteProgram(it->second.id);
    auto createInfos = it->second.createInfos;
    storage->shaders.erase(it);

    auto it2 = storage->shaders.emplace(name, std::move(data));
    return Shader(it2.first->second.uniformIDs, it2.first->second.id);
  }


  std::vector<std::string> ShaderManager::GetAllShaderNames()
  {
    std::vector<std::string> names;
    names.reserve(storage->shaders.size());
    for (const auto& [id, data] : storage->shaders)
    {
      names.push_back(data.name);
    }
    return names;
  }
}