#pragma once
#include "utilities.h"
#include <unordered_map>
#include <optional>
#include <engine_assert.h>

#include <Graphics/GraphicsIncludes.h>
#include <shaderc/shaderc.hpp>

#pragma warning(push)
#pragma warning(disable : 4267) // 8->4 byte int conversion

// encapsulates shaders by storing uniforms and its GPU memory location
// also stores the program's name and both shader paths for recompiling
typedef class Shader
{
public:
	using glShaderType = GLint;

	GLuint programID;		// the program's address in GPU memory
	const int shaderID;	// index into shader array
	std::string name;		// probably actual index into shader array
	std::unordered_map<std::string, GLint> Uniforms;

	std::string vsPath;	// vertex shader path
	std::string tcsPath;// tessellation control shader path
	std::string tesPath;// tessellation evaluation shader path
	std::string gsPath;	// geometry shader path
	std::string fsPath;	// fragment shader path
	std::string csPath;	// compute shader path

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
	Shader() : shaderID(shader_count_)
	{
		//type = sDefault;
		programID = NULL;
		shader_count_++;
	}

	~Shader()
	{
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

	void setBool(const char* uniform, bool value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1i(programID, Uniforms[uniform], (int)value);
	}
	void setInt(const char* uniform, int value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1i(programID, Uniforms[uniform], value);
	}
	void setUInt(const char* uniform, int value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1ui(programID, Uniforms[uniform], value);
	}
	void setFloat(const char* uniform, float value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1f(programID, Uniforms[uniform], value);
	}
	void set1FloatArray(const char* uniform, const std::vector<float>& value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1fv(programID, Uniforms[uniform], value.size(), &value[0]);
	}
	void set1FloatArray(const char* uniform, const float* value, GLsizei count)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1fv(programID, Uniforms[uniform], count, value);
	}
	void set2FloatArray(const char* uniform, const std::vector<glm::vec2>& value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform2fv(programID, Uniforms[uniform], value.size(), &value[0].x);
	}
	void set3FloatArray(const char* uniform, const std::vector<glm::vec3>& value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform3fv(programID, Uniforms[uniform], value.size(), &value[0].x);
	}
	void set4FloatArray(const char* uniform, const std::vector<glm::vec4>& value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform4fv(programID, Uniforms[uniform], value.size(), &value[0].x);
	}
	void setIntArray(const char* uniform, const std::vector<int>& value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform1iv(programID, Uniforms[uniform], value.size(), &value[0]);
	}
	void setVec2(const char* uniform, const glm::vec2 &value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform2fv(programID, Uniforms[uniform], 1, &value[0]);
	}
	void setVec2(const char* uniform, float x, float y)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform2f(programID, Uniforms[uniform], x, y);
	}
	void setVec3(const char* uniform, const glm::vec3 &value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform3fv(programID, Uniforms[uniform], 1, &value[0]);
	}
	void setVec3(const char* uniform, float x, float y, float z)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform3f(programID, Uniforms[uniform], x, y, z);
	}
	void setVec4(const char* uniform, const glm::vec4 &value)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform4fv(programID, Uniforms[uniform], 1, &value[0]);
	}
	void setVec4(const char* uniform, float x, float y, float z, float w)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniform4f(programID, Uniforms[uniform], x, y, z, w);
	}
	void setMat3(const char* uniform, const glm::mat3 &mat)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniformMatrix3fv(programID, Uniforms[uniform], 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const char* uniform, const glm::mat4 &mat)
	{
		ASSERT(Uniforms.find(uniform) != Uniforms.end());
		glProgramUniformMatrix4fv(programID, Uniforms[uniform], 1, GL_FALSE, &mat[0][0]);
	}

	// list of all shader programs
	static std::unordered_map<std::string, Shader*> shaders;
private:
	enum shadertype : GLint
	{
		TY_VERTEX = GL_VERTEX_SHADER,
		TY_TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		TY_TESS_EVAL = GL_TESS_EVALUATION_SHADER,
		TY_GEOMETRY = GL_GEOMETRY_SHADER,
		TY_FRAGMENT = GL_FRAGMENT_SHADER,
		TY_COMPUTE = GL_COMPUTE_SHADER,
	};

	static int shader_count_;

	friend class IncludeHandler;

	// shader dir includes source and headers alike
	static constexpr const char* shader_dir_ = "./resources/Shaders/";
	static std::string loadFile(std::string path);

	GLint compileShader(shadertype type, const std::vector<std::string>& src);
	void initUniforms();
	void checkLinkStatus(std::vector<std::string_view> files);

	// returns compiled SPIR-V
	std::vector<uint32_t>
		spvPreprocessAndCompile(
			shaderc::Compiler& compiler,
			const shaderc::CompileOptions options,
			std::string path,
			shaderc_shader_kind a);
}Shader, *ShaderPtr;

#pragma warning(pop)