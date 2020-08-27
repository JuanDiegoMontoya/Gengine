#include <Systems/Graphics/GraphicsIncludes.h>
#include <sstream>
#include <fstream>
#include <iostream>

int Shader::shader_count_ = 0;
std::unordered_map<std::string, Shader*> Shader::shaders = std::unordered_map<std::string, Shader*>();

// the provided path does not need to include the shader directory
Shader::Shader(const char* vertexPath,
               const char* fragmentPath,
               const char* tessCtrlPath,
               const char* tessEvalPath,
               const char* geometryPath) 
	: shaderID(shader_count_++)
{
	vsPath = vertexPath;
	fsPath = fragmentPath;
	tcsPath = tessCtrlPath;
	tesPath = tessEvalPath;
	gsPath = geometryPath;
	const std::string vertSrc = loadShader(vertexPath).c_str();
	const std::string fragSrc = loadShader(fragmentPath).c_str();

	// compile individual shaders
	programID = glCreateProgram();
	GLint vShader = compileShader(TY_VERTEX, vertSrc.c_str());
	GLint fShader = compileShader(TY_FRAGMENT, fragSrc.c_str());
	GLint tcShader = 0;
	GLint teShader = 0;
	GLint gShader  = 0;

	if (strcmp(tessCtrlPath, "<null>"))
	{
		tcShader = compileShader(TY_TESS_CONTROL, loadShader(tessCtrlPath).c_str());
		glAttachShader(programID, tcShader);
	}
	if (strcmp(tessEvalPath, "<null>"))
	{
		teShader = compileShader(TY_TESS_EVAL, loadShader(tessEvalPath).c_str());
		glAttachShader(programID, teShader);
	}
	if (strcmp(geometryPath, "<null>"))
	{
		gShader = compileShader(TY_GEOMETRY, loadShader(geometryPath).c_str());
		glAttachShader(programID, gShader);
	}

	// vertex and fragment shaders are required (technically not frag but we want it to be here)
	glAttachShader(programID, vShader);
	glAttachShader(programID, fShader);
	glLinkProgram(programID);

	checkLinkStatus({ vertexPath, fragmentPath });

	glDeleteShader(vShader);
	glDeleteShader(fShader);
	if (strcmp(tessCtrlPath, "<null>"))
		glDeleteShader(tcShader);
	if (strcmp(tessEvalPath, "<null>"))
		glDeleteShader(teShader);
	if (strcmp(geometryPath, "<null>"))
		glDeleteShader(gShader);

	initUniforms();
}

Shader::Shader(const char* computePath) : shaderID(shader_count_++)
{
	csPath = computePath;
	const std::string compSrc = loadShader(computePath).c_str();
	programID = glCreateProgram();
	GLint cShader = compileShader(TY_COMPUTE, compSrc.c_str());
	glAttachShader(programID, cShader);
	glLinkProgram(programID);
	checkLinkStatus({ computePath });
	glDeleteShader(cShader);

	initUniforms();
}

// loads a shader source into a string
std::string Shader::loadShader(const char* path)
{
	std::string shaderpath = std::string(shader_dir_) + path;
	std::string content;
	try
	{
		std::ifstream ifs(shaderpath);
		content = std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Error reading shader: " << path << '\n';
		std::cout << "Message: " << e.what() << std::endl;
	}
	return std::string(content);
}

// compiles a shader source and returns its ID
GLint Shader::compileShader(shadertype type, const GLchar* src)
{
	GLuint shader = 0;
	GLchar infoLog[512];
	std::string path;
	GLint success;
	
	switch (type)
	{
	case Shader::TY_VERTEX:
		shader = glCreateShader(GL_VERTEX_SHADER);
		path = vsPath;
		break;
	case Shader::TY_TESS_CONTROL:
		shader = glCreateShader(GL_TESS_CONTROL_SHADER);
		path = tcsPath;
		break;
	case Shader::TY_TESS_EVAL:
		shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		path = vsPath;
		break;
	case Shader::TY_GEOMETRY:
		shader = glCreateShader(GL_GEOMETRY_SHADER);
		path = vsPath;
		break;
	case Shader::TY_FRAGMENT:
		shader = glCreateShader(GL_FRAGMENT_SHADER);
		path = fsPath;
		break;
	case Shader::TY_COMPUTE:
		shader = glCreateShader(GL_COMPUTE_SHADER);
		path = csPath;
		break;
	default:
		path = nullptr;
		break;
	}

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "File: " << path << std::endl;
		std::cout << "Error compiling shader of type " << type << '\n' << infoLog << std::endl;
	}
	else
	{
		// compile successful
	}

	return shader;
}

// TODO: https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions#ideal-way-of-retrieving-all-uniform-names
void Shader::initUniforms()
{
	// init uniform map used in that shader
	GLint max_length;
	GLint num_uniforms;

	glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
	//GLchar* pname = (GLchar*)alloca(max_length * sizeof(GLchar));
	GLchar* pname = new GLchar[max_length];
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &num_uniforms);

	for (GLint i = 0; i < num_uniforms; ++i)
	{
		GLsizei written;
		GLint size;
		GLenum type;

		glGetActiveUniform(programID, i, max_length, &written, &size, &type, pname);
		GLchar* pname1 = new GLchar[max_length];
		std::strcpy(pname1, pname);
		if (size > 1)
			pname1[written - 3] = '\0';
		GLint loc = glGetUniformLocation(programID, pname1);
		Uniforms.insert(std::pair<GLchar*, GLint>(pname1, loc));
		//delete pname1;
	}

	// unfortunately we must have this in the same scope as where it's constructed
	// alloca prevents the use of delete, but its implementation is compiler dependent
	delete[] pname;
}

void Shader::checkLinkStatus(std::vector<std::string> files)
{
	// link program
	GLint success;
	GLchar infoLog[512];
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, NULL, infoLog);
		std::cout << "File(s): ";// << vertexPath << ", " << fragmentPath << '\n';
		for (const auto& file : files)
			std::cout << file << (file == *(files.end() - 1) ? "" : ", "); // no comma on last element
		std::cout << '\n';
		std::cout << "Failed to link shader program\n" << infoLog << std::endl;
		//traceMessage(std::string(infoLog));
	}
	else
	{
		// link successful
	}
}
