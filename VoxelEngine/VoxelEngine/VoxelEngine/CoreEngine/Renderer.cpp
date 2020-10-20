#include "Renderer.h"

#include <CoreEngine/WindowUtils.h>
#include <CoreEngine/Window.h>
#include <CoreEngine/shader.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Camera.h>
#include <CoreEngine/Texture2D.h>

static void GLAPIENTRY
GLerrorCB(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	[[maybe_unused]] const void* userParam)
{
	//return; // UNCOMMENT WHEN DEBUGGING GRAPHICS

	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204
		)//|| id == 131188 || id == 131186)
		return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window Manager"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

void Renderer::Render(Components::Transform& model, Components::Mesh& mesh, Components::Material& mat)
{
	auto& material = MaterialManager::materials[mat];
	auto& shader = Shader::shaders[material.shaderID];
	shader->Use();

	glm::mat4 modelMatrix = model.GetModel();
	modelMatrix = glm::scale(modelMatrix, { 10, 10, 10 });
	glm::mat4 modelInv = glm::inverse(modelMatrix);
	modelInv = glm::transpose(modelInv);

	glm::mat4 MVP = Camera::ActiveCamera->GetProjView() * modelMatrix;

	//ShadershadersShaderMcShaderFuckFaceUse->setMat4("InvTrModel", modelInv);
	shader->setMat4("MVP", MVP);
	//ShadershadersShaderMcShaderFuckFaceUse->setMat4("Model", modelMatrix);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, mat.texHandle);
	int i = 0;
	for (const auto& tex : material.textures)
	{
		tex.Bind(i++);
	}
	shader->setInt("albedoMap", 0);

	MeshHandle mHandle = mesh.meshHandle;

	glBindVertexArray(mHandle.VAO);
	glDrawElements(GL_TRIANGLES, (int)mHandle.indexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::Init()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEPTH_TEST);

	// enable debugging stuff
	glDebugMessageCallback(GLerrorCB, NULL);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glfwSwapInterval(0);

	CompileShaders();


	/*Layout layout = Window::layout;

	int width = layout.width;
	int height = layout.height;

	// Setup frameBuffer
	glGenFramebuffers(1, &RenderBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, RenderBuffer);

	// Setup render texture
	glGenTextures(1, &RenderTexture);
	glBindTexture(GL_TEXTURE_2D, RenderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer in Raytrace Renderer is not complete!" << std::endl;
	}*/
}

void Renderer::CompileShaders()
{
	Shader::shaders["ShaderMcShaderFuckFace"].emplace(Shader(
		{
			{ "TexturedMesh.vs", GL_VERTEX_SHADER },
			{ "TexturedMesh.fs", GL_FRAGMENT_SHADER }
		}));

	Shader::shaders["chunk_optimized"].emplace(Shader(
		{
			{ "chunk_optimized.vs", GL_VERTEX_SHADER },
			{ "chunk_optimized.fs", GL_FRAGMENT_SHADER }
		}));
	//Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
	Shader::shaders["compact_batch"].emplace(Shader(
		{ { "compact_batch.cs", GL_COMPUTE_SHADER } }));
	//Shader::shaders["compact_batch"] = new Shader(0, "compact_batch.cs");
	Shader::shaders["textured_array"].emplace(Shader(
		{
			{ "textured_array.vs", GL_VERTEX_SHADER },
			{ "textured_array.fs", GL_FRAGMENT_SHADER }
		}));
	Shader::shaders["buffer_vis"].emplace(Shader(
		{
			{ "buffer_vis.fs", GL_FRAGMENT_SHADER },
			{ "buffer_vis.vs", GL_VERTEX_SHADER }
		}));
	Shader::shaders["chunk_render_cull"].emplace(Shader(
		{
			{ "chunk_render_cull.vs", GL_VERTEX_SHADER },
			{ "chunk_render_cull.fs", GL_FRAGMENT_SHADER }
		}));

	Shader::shaders["sun"].emplace(Shader("flat_sun.vs", "flat_sun.fs"));
	Shader::shaders["axis"].emplace(Shader("axis.vs", "axis.fs"));
	Shader::shaders["flat_color"].emplace(Shader("flat_color.vs", "flat_color.fs"));
}

void Renderer::DrawAxisIndicator()
{
	static VAO* axisVAO;
	static StaticBuffer* axisVBO;
	if (axisVAO == nullptr)
	{
		float indicatorVertices[] =
		{
			// positions      // colors
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
			1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};

		axisVAO = new VAO();
		axisVBO = new StaticBuffer(indicatorVertices, sizeof(indicatorVertices));
		VBOlayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		axisVAO->AddBuffer(*axisVBO, layout);
	}
	auto& currShader = Shader::shaders["axis"];
	currShader->Use();
	Camera* cam = Camera::ActiveCamera;
	currShader->setMat4("u_model", glm::translate(glm::mat4(1), cam->GetPos() + cam->GetFront() * 10.f)); // add scaling factor (larger # = smaller visual)
	currShader->setMat4("u_view", cam->GetView());
	currShader->setMat4("u_proj", cam->GetProj());
	glClear(GL_DEPTH_BUFFER_BIT); // allows indicator to always be rendered
	axisVAO->Bind();
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, 6);
	axisVAO->Unbind();
}

// draws a single quad over the entire viewport
void Renderer::DrawQuad()
{
	static unsigned int quadVAO = 0;
	static unsigned int quadVBO;
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Renderer::DrawCube()
{
	static VAO* blockHoverVao = nullptr;
	static StaticBuffer* blockHoverVbo = nullptr;
	if (blockHoverVao == nullptr)
	{
		blockHoverVao = new VAO();
		blockHoverVbo = new StaticBuffer(Vertices::cube_norm_tex, sizeof(Vertices::cube_norm_tex));
		VBOlayout layout;
		layout.Push<float>(3);
		layout.Push<float>(3);
		layout.Push<float>(2);
		blockHoverVao->AddBuffer(*blockHoverVbo, layout);
	}
	blockHoverVao->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
}