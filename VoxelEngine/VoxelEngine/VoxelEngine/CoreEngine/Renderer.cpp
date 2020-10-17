#include "Renderer.h"

#include "WindowUtils.h"
#include "Window.h"
#include "../Graphics/shader.h"
#include "Components.h"
#include "Camera.h"
#include "Texture2D.h"

using namespace Components;

void Renderer::Render(Model& model, Mesh& mesh, Material& mat)
{
	auto& material = MaterialManager::materials[mat];
	auto& shader = Shader::shaders[material.shaderID];
	shader->Use();

	glm::mat4 modelMatrix = model.model;
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
	Shader::shaders["ShaderMcShaderFuckFace"].emplace(Shader(
		{
		  { "TexturedMesh.vs", GL_VERTEX_SHADER },
		  { "TexturedMesh.fs", GL_FRAGMENT_SHADER }
		}));
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