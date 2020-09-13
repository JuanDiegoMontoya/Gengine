#include "stdafx.h"
#include "NuRenderer.h"
#include "Renderer.h" // for old rendering functions
#include "World.h"
#include <Engine.h>
#include <shader.h>
#include <camera.h>
#include <input.h>
#include "ChunkStorage.h"
#include <dib.h>
#include "ChunkRenderer.h"
#include "block.h"
#include "TextureArray.h"
#include <texture.h>
#include "sun.h"
#include "RenderOrder.h"

namespace NuRenderer
{
	namespace
	{
		// block textures
		std::unique_ptr<TextureArray> textures;
		std::unique_ptr<Texture> blueNoise64;
	}


	TextureArray* GetBlockTextures()
	{
		return textures.get();
	}


	void Init()
	{
		std::vector<std::string> texs;
		for (const auto& prop : Block::PropertiesTable)
		{
			texs.push_back(std::string(prop.name) + ".png");
		}
		textures = std::make_unique<TextureArray>(texs);

		//blueNoise64 = std::make_unique<Texture>("BlueNoise/64_64/LDR_LLL1_0.png");
		blueNoise64 = std::make_unique<Texture>("BlueNoise/256_256/LDR_LLL1_0.png");

		CompileShaders();
		Engine::PushRenderCallback(DrawAll, RenderOrder::RenderDrawAll);
		Engine::PushRenderCallback(ChunkRenderer::Update, RenderOrder::RenderChunkRenderUpdate);
		Engine::PushRenderCallback(Clear, RenderOrder::RenderClear);
		Engine::PushRenderCallback(Renderer::drawSky, RenderOrder::RenderSky);
	}


	void CompileShaders()
	{
		Shader::shaders["chunk_optimized"] = new Shader(
			{
				{ "chunk_optimized.vs", GL_VERTEX_SHADER },
				{ "chunk_optimized.fs", GL_FRAGMENT_SHADER }
			});
		//Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
		Shader::shaders["compact_batch"] = new Shader(
			{ { "compact_batch.cs", GL_COMPUTE_SHADER } });
		//Shader::shaders["compact_batch"] = new Shader(0, "compact_batch.cs");
		Shader::shaders["textured_array"] = new Shader(
			{
				{ "textured_array.vs", GL_VERTEX_SHADER },
				{ "textured_array.fs", GL_FRAGMENT_SHADER }
			});
		Shader::shaders["buffer_vis"] = new Shader(
			{
				{ "buffer_vis.fs", GL_FRAGMENT_SHADER },
				{ "buffer_vis.vs", GL_VERTEX_SHADER }
			});
		Shader::shaders["chunk_render_cull"] = new Shader(
			{
				{ "chunk_render_cull.vs", GL_VERTEX_SHADER },
				{ "chunk_render_cull.fs", GL_FRAGMENT_SHADER }
			});
		//Shader::shaders["chunk_render_cull"] = new Shader("chunk_render_cull.vs", "chunk_render_cull.fs");
	}


	void Clear()
	{
		drawCalls = 0;
		auto cc = World::bgColor_;
		glClearColor(cc.r, cc.g, cc.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}


	void DrawAll()
	{
		PERF_BENCHMARK_START;

		if (settings.gammaCorrection)
			glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

		if (Input::Keyboard().down[GLFW_KEY_LEFT_SHIFT])
		{
			if (Input::Keyboard().down[GLFW_KEY_1])
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if (Input::Keyboard().down[GLFW_KEY_2])
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			if (Input::Keyboard().down[GLFW_KEY_3])
				glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		}

		drawChunks();
		//splatChunks();
		drawChunksWater();
		Renderer::drawAxisIndicators();
		ChunkRenderer::DrawBuffers();
		//Renderer::postProcess();

		glDisable(GL_FRAMEBUFFER_SRGB);

		PERF_BENCHMARK_END;
	}


	void drawChunks()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // don't forget to reset original culling face

		// render blocks in each active chunk
		ShaderPtr currShader = Shader::shaders["chunk_optimized"];
		currShader->Use();

		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		float angle = glm::max(glm::dot(-glm::normalize(Renderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
		currShader->setFloat("sunAngle", angle);
		//printf("Angle: %f\n", angle);
		// currShader->setInt("textureAtlas", ...);

		float loadD = World::chunkManager_.GetLoadDistance();
		float loadL = World::chunkManager_.GetUnloadLeniency();
		// undo gamma correction for sky color
		static glm::vec3 skyColor(
			glm::pow(.529f, 2.2f),
			glm::pow(.808f, 2.2f),
			glm::pow(.922f, 2.2f));
		currShader->setVec3("viewPos", cam->GetPos());
		//currShader->setFloat("fogStart", loadD - loadD / 2.f);
		//currShader->setFloat("fogEnd", loadD - Chunk::CHUNK_SIZE * 1.44f); // cuberoot(3)
		currShader->setFloat("fogStart", settings.fogStart);
		currShader->setFloat("fogEnd", settings.fogEnd);
		currShader->setVec3("fogColor", skyColor);
		currShader->setMat4("u_viewProj", cam->GetProj() * cam->GetView());

		textures->Bind(0);
		currShader->setInt("textures", 0);
		blueNoise64->Bind(1);
		currShader->setInt("blueNoise", 1);



		currShader->Use();
		//ChunkRenderer::RenderNorm();
		ChunkRenderer::Render();
		ChunkRenderer::GenerateDIB();
		ChunkRenderer::RenderOcclusion();
		drawCalls++;
		return;






		auto& chunks = ChunkStorage::GetMapRaw();
		auto it = chunks.begin();
		auto end = chunks.end();
		for (int i = 0; it != end; ++it)
		{
			ChunkPtr chunk = it->second;
			if (chunk 
				//&& chunk->IsVisible(*cam) 
				//&& glm::distance(cam->GetPos(), glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE)) < 200
				)
			{
				// set some uniforms, etc
				//currShader->setVec3("u_pos", Chunk::CHUNK_SIZE * chunk->GetPos());
				chunk->Render();
				drawCalls++;
			}
		}
	}


	void splatChunks()
	{
		glEnable(GL_PROGRAM_POINT_SIZE);
		Camera* cam = Renderer::GetPipeline()->GetCamera(0);
		const auto& proj = cam->GetProj();
		const auto& view = cam->GetView();
		ShaderPtr currShader = Shader::shaders["chunk_splat"];
		currShader->Use();

		currShader->setVec3("u_viewpos", cam->GetPos());

		currShader->setMat4("u_viewProj", proj * view);
		currShader->setMat4("u_invProj", glm::inverse(proj));
		currShader->setMat4("u_invView", glm::inverse(view));

		float angle = glm::max(glm::dot(-glm::normalize(Renderer::activeSun_->GetDir()), glm::vec3(0, 1, 0)), 0.f);
		currShader->setFloat("sunAngle", angle);

		GLint vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);
		currShader->setVec2("u_viewportSize", glm::vec2(vp[2], vp[3]));





		//ChunkRenderer::GenerateDrawCommandsSplat();
		ChunkRenderer::GenerateDrawCommandsSplatGPU();
		currShader->Use();
		ChunkRenderer::RenderSplat();
		drawCalls++;
		return;






		std::for_each(ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end(),
			[&](const std::pair<glm::ivec3, Chunk*>& pair)
		{
			ChunkPtr chunk = pair.second;
			if (//chunk && chunk->IsVisible(*cam) &&
				glm::distance(cam->GetPos(), glm::vec3(chunk->GetPos() * Chunk::CHUNK_SIZE)) >= 200)
			{
				// set some uniforms, etc
				currShader->setVec3("u_pos", Chunk::CHUNK_SIZE * chunk->GetPos());
				chunk->RenderSplat();
				drawCalls++;
			}
		});
	}


	void drawChunksWater()
	{

	}
}