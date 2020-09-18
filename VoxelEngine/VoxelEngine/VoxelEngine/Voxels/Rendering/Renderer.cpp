#include "stdafx.h"
#include "chunk.h"
#include "vbo.h"
#include "vao.h"
#include "ibo.h"
#include "shader.h"
#include "directional_light.h"
#include "camera.h"
#include "pipeline.h"
#include "settings.h"
#include "sun.h"
#include "input.h"
#include "chunk_manager.h"

#include "Renderer.h"
#include <Vertices.h>
#include <Engine.h>
#include "World.h"

#define VISUALIZE_MAPS 0


// initializes the gBuffer and its attached textures
namespace Renderer
{
	// private variables
	namespace
	{
		Pipeline pipeline;
	}


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
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
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


	void CompileShaders()
	{
		// initialize all of the shaders that will be used
		//Shader::shaders["postprocess"] = new Shader("postprocess.vs", "postprocess.fs");
		//Shader::shaders["flat"] = new Shader("flat_color_instanced.vs", "flat_color_instanced.fs");
		//Shader::shaders["chunk"] = new Shader("chunk_flat.vs", "chunk_flat.fs");
		Shader::shaders["sun"] = new Shader("flat_sun.vs", "flat_sun.fs");
		//Shader::shaders["shadow"] = new Shader("shadow.vs", "shadow.fs");
		Shader::shaders["axis"] = new Shader("axis.vs", "axis.fs");
		Shader::shaders["flat_color"] = new Shader("flat_color.vs", "flat_color.fs");

		//Shader::shaders["chunk_water"] = new Shader("chunk_water.vs", "chunk_water.fs");
		//Shader::shaders["chunk_shaded"] = new Shader("chunk_smooth_light.vs", "chunk_smooth_light.fs");
		//Shader::shaders["chunk_geometry"] = new Shader("chunk_gBuffer.vs", "chunk_gBuffer.fs");
		//std::vector<int> values = { 0, 1, 2 };
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[0]", 0);
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[1]", 1);
		//Shader::shaders["chunk_shaded"]->setInt("shadowMap[2]", 2);
		//Shader::shaders["chunk_shaded"]->Use();
		//Shader::shaders["chunk_shaded"]->setIntArray("shadowMap", values);
		//Shader::shaders["chunk_water"]->Use();
		//Shader::shaders["chunk_water"]->setIntArray("shadowMap", values);
		//Shader::shaders["chunk_water"]->setInt("ssr_positions", 3);
		//Shader::shaders["chunk_water"]->setInt("ssr_normals", 4);
		//Shader::shaders["chunk_water"]->setInt("ssr_albedoSpec", 5);
		//Shader::shaders["chunk_water"]->setInt("ssr_depth", 6);


		//Shader::shaders["debug_map3"] = new Shader("debug_map.vs", "debug_map.fs");
		//Shader::shaders["debug_shadow"] = new Shader("debug_shadow.vs", "debug_shadow.fs");
		//Shader::shaders["debug_shadow"]->Use();
		//Shader::shaders["debug_shadow"]->setInt("depthMap", 1);
	}


	void RecompileShaders()
	{
		for (auto& shader : Shader::shaders)
		{
			//if (shader.first == "postprocess")
			//	continue;
			std::string vs = shader.second->vsPath.c_str();
			std::string fs = shader.second->fsPath.c_str();
			delete shader.second;
			shader.second = new Shader(vs.c_str(), fs.c_str());
		}
	}

	// initializes the gBuffer and its attached textures
	void Init()
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

		initDeferredBuffers();
		initPPBuffers();

		GLint count;
		glGetIntegerv(GL_NUM_EXTENSIONS, &count);

		for (GLint i = 0; i < count; ++i)
		{
			const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
			if (strcmp(extension, "GL_NVX_gpu_memory_info") == 0)
				nvUsageEnabled = true;
		}

		CompileShaders();
		//pipeline.AddCamera()

		//Engine::PushRenderCallback(DrawAll, 0);
		//Engine::PushUpdateCallback(Update, 0);
	}

	// draws everything at once, I guess?
	// this should be fine
	void DrawAll()
	{
		PERF_BENCHMARK_START;

		Clear();
		glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction

		//std::cout << "RENDER" << std::endl;

		//glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);
		drawAxisIndicators();
		drawDepthMapsDebug();

		//glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);

		postProcess();
		//drawPostProcessing();

		glDisable(GL_FRAMEBUFFER_SRGB);
		PERF_BENCHMARK_END;
	}

	void Clear()
	{
		auto cc = World::bgColor_;
		glClearColor(cc.r, cc.g, cc.b, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void ClearCSM()
	{
		for (int i = 0; i < activeDirLight_->GetNumCascades(); i++)
		{
			activeDirLight_->bindForWriting(i);
			glClear(GL_DEPTH_BUFFER_BIT);
			glClearTexSubImage(
				activeDirLight_->GetDepthTex()[i], 0, 0, 0, 0,
				activeDirLight_->GetShadowSize().x, activeDirLight_->GetShadowSize().y, 0,
				GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void ClearGGuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// OpenGL 4.4+
		int scrX = Settings::Graphics.screenX;
		int scrY = Settings::Graphics.screenY;
		glClearTexSubImage(gPosition, 0, 0, 0, 0, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
		glClearTexSubImage(gNormal, 0, 0, 0, 0, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
		glClearTexSubImage(gAlbedoSpec, 0, 0, 0, 0, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glClearTexSubImage(gDepth, 0, 0, 0, 0, scrX, scrY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	void SetDirLight(DirLight* d)
	{
		activeDirLight_ = d;
	}
	void SetSun(Sun* s)
	{
		activeSun_ = s;
	}

	void DrawCube()
	{
		static VAO* blockHoverVao = nullptr;
		static VBO* blockHoverVbo = nullptr;
		if (blockHoverVao == nullptr)
		{
			blockHoverVao = new VAO();
			blockHoverVbo = new VBO(Vertices::cube_norm_tex, sizeof(Vertices::cube_norm_tex));
			VBOlayout layout;
			layout.Push<float>(3);
			layout.Push<float>(3);
			layout.Push<float>(2);
			blockHoverVao->AddBuffer(*blockHoverVbo, layout);
		}
		//glClear(GL_DEPTH_BUFFER_BIT);
		//glDisable(GL_CULL_FACE);
		blockHoverVao->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glEnable(GL_CULL_FACE);
	}


	// draw sky objects (then clear the depth buffer)
	void drawSky()
	{
		activeSun_->Render();
	}

	// all post processing effects
	void drawPostProcessing()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind default framebuffer
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int scrX = Settings::Graphics.screenX;
		int scrY = Settings::Graphics.screenY;

		ShaderPtr shader = Shader::shaders["postprocess"];
		shader->Use();
		shader->setInt("colorMap", 0);
		shader->setInt("depthMap", 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pDepth);
		//shader->setFloat("near_plane", Render::GetCamera()->GetNear());
		//shader->setFloat("far_plane", Render::GetCamera()->GetFar());

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FRAMEBUFFER_SRGB);
		drawQuad();

		glEnable(GL_DEPTH_TEST);
	}


	//void Renderer::drawDebug()
	//{
	//	std::for_each(Chunk::chunks.begin(), Chunk::chunks.end(),
	//		[&](std::pair<glm::ivec3, Chunk*> chunk)
	//		{
	//			if (chunk.second && (cullFrustum ? chunk.second->IsVisible() : true))
	//			{
	//				auto model = chunk.second->GetModel();
	//				glDisable(GL_CULL_FACE);
	//				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//				model = glm::translate(model, { 1,1,1 });
	//				draw_cb(glm::scale(model, { 32, 32, 32 }));
	//				DrawCube();
	//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//				glEnable(GL_CULL_FACE);
	//			}
	//		});
	//}


	void drawBillboard(VAO* vao, size_t count, DrawCB uniform_cb)
	{
		// TODO: set this up for actual use
	}

	void drawDepthMapsDebug()
	{
		Shader::shaders["debug_shadow"]->Use();
		Shader::shaders["debug_shadow"]->setFloat("near_plane",GetPipeline()->GetCamera(0)->GetNear());
		Shader::shaders["debug_shadow"]->setFloat("far_plane", GetPipeline()->GetCamera(0)->GetFar());

		// debug shadows
		if (Input::Keyboard().down[GLFW_KEY_4])
		{
			Shader::shaders["debug_shadow"]->Use();
			Shader::shaders["debug_shadow"]->setInt("perspective", 0);
			for (int i = 0; i < 3; i++)
			{
				glViewport(0 + 512 * i, 0, 512, 512);
				Shader::shaders["debug_shadow"]->setInt("depthMap", i);
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, activeDirLight_->GetDepthTex()[i]);
				drawQuad();
			}
		}

		if (Input::Keyboard().down[GLFW_KEY_5])
		{
			Shader::shaders["debug_map3"]->Use();
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

			Shader::shaders["debug_map3"]->setInt("map", 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
			drawQuad();
		}

		if (Input::Keyboard().down[GLFW_KEY_8])
		{
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
			Shader::shaders["debug_shadow"]->Use();
			Shader::shaders["debug_shadow"]->setInt("depthMap", 3);
			Shader::shaders["debug_shadow"]->setInt("perspective", 1);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, gDepth);

			drawQuad();
		}

		if (Input::Keyboard().down[GLFW_KEY_9])
		{
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
			Shader::shaders["debug_map3"]->Use();
			Shader::shaders["debug_map3"]->setInt("map", 4);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pColor);

			drawQuad();
		}

		if (Input::Keyboard().down[GLFW_KEY_0])
		{
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);
			Shader::shaders["debug_shadow"]->Use();
			Shader::shaders["debug_shadow"]->setInt("depthMap", 5);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, pDepth);

			drawQuad();
		}

		// TODO: use the good deferred pipeline stuff so we can get rid of this filthy macro
#if VISUALIZE_MAPS
		if (Input::Keyboard().down[GLFW_KEY_6])
		{
			Shader::shaders["debug_map3"]->Use();
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

			Shader::shaders["debug_map3"]->setInt("map", 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			drawQuad();
		}

		if (Input::Keyboard().down[GLFW_KEY_7])
		{
			Shader::shaders["debug_map3"]->Use();
			glViewport(0, 0, Settings::Graphics.screenX, Settings::Graphics.screenY);

			Shader::shaders["debug_map3"]->setInt("map", 2);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			drawQuad();
		}

#endif
	}

	void drawAxisIndicators()
	{
		static VAO* axisVAO;
		static VBO* axisVBO;
		if (axisVAO == nullptr)
		{
			float indicatorVertices[] =
			{
				// positions			// colors
				0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // x-axis
				1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // y-axis
				0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // z-axis
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
			};

			axisVAO = new VAO();
			axisVBO = new VBO(indicatorVertices, sizeof(indicatorVertices), GL_STATIC_DRAW);
			VBOlayout layout;
			layout.Push<float>(3);
			layout.Push<float>(3);
			axisVAO->AddBuffer(*axisVBO, layout);
		}
		/* Renders the axis indicator (a screen-space object) as though it were
			one that exists in the world for simplicity. */
		ShaderPtr currShader = Shader::shaders["axis"];
		currShader->Use();
		Camera* cam = GetPipeline()->GetCamera(0);
		currShader->setMat4("u_model", glm::translate(glm::mat4(1), cam->GetPos() + cam->front * 10.f)); // add scaling factor (larger # = smaller visual)
		currShader->setMat4("u_view", cam->GetView());
		currShader->setMat4("u_proj", cam->GetProj());
		glClear(GL_DEPTH_BUFFER_BIT); // allows indicator to always be rendered
		axisVAO->Bind();
		glLineWidth(2.f);
		glDrawArrays(GL_LINES, 0, 6);
		axisVAO->Unbind();
	}

	void initDeferredBuffers()
	{
		int scrX = Settings::Graphics.screenX;
		int scrY = Settings::Graphics.screenY;

		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// 'color' indicates that the data is stored in a color format
		// position 'color' buffer
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
#if VISUALIZE_MAPS
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

		// normal 'color' buffer
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
#if VISUALIZE_MAPS
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrX, scrY, 0, GL_RGB, GL_FLOAT, NULL);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// color + specular 'color' buffer
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

		// depth attachment
		glGenTextures(1, &gDepth);
		glBindTexture(GL_TEXTURE_2D, gDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, scrX, scrY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);

		unsigned int attachments[4] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2, GL_DEPTH_ATTACHMENT };
		glDrawBuffers(3, attachments);

		// attach depth renderbuffer
		//glGenRenderbuffers(1, &rboDepth);
		//glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scrX, scrY);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// lite up dat mf'in geometry we got there
	void lightingPass()
	{
	}

	void initPPBuffers()
	{
		int scrX = Settings::Graphics.screenX;
		int scrY = Settings::Graphics.screenY;

		glGenFramebuffers(1, &pBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);

		// color buffer
		glGenTextures(1, &pColor);
		glBindTexture(GL_TEXTURE_2D, pColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scrX, scrY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pColor, 0);

		// depth attachment
		glGenTextures(1, &pDepth);
		glBindTexture(GL_TEXTURE_2D, pDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, scrX, scrY, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, pDepth, 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void postProcess()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pBuffer);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int scrX = Settings::Graphics.screenX;
		int scrY = Settings::Graphics.screenY;

		// copy default framebuffer contents into pBuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pBuffer);
		//glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(
			0, 0, scrX, scrY,
			0, 0, scrX, scrY,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		//glBlitFramebuffer(
		//	0, 0, scrX / 2, scrY / 2,
		//	0, 0, scrX / 2, scrY / 2,
		//	GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pColor);
		ShaderPtr shader = Shader::shaders["postprocess"];
		shader->Use();

		shader->setInt("colorTex", 0);
		shader->setBool("sharpenFilter", ppSharpenFilter);
		shader->setBool("edgeDetection", ppEdgeDetection);
		shader->setBool("chromaticAberration", ppChromaticAberration);
		shader->setBool("blurFilter", ppBlurFilter);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FRAMEBUFFER_SRGB);
		drawQuad();
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_DEPTH_TEST);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// copy pBuffer contents into default framebuffer
		//glBindFramebuffer(GL_READ_FRAMEBUFFER, pBuffer);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		//glBlitFramebuffer(
		//	0, 0, scrX / 2, scrY / 2,
		//	0, 0, scrX / 2, scrY / 2,
		//	GL_COLOR_BUFFER_BIT, GL_LINEAR);
		//glBlitFramebuffer(
		//	0, 0, scrX / 2, scrY / 2,
		//	0, 0, scrX / 2, scrY / 2,
		//	GL_DEPTH_BUFFER, GL_NEAREST); // must be GL_NEAREST for depth
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glEnable(GL_DEPTH_TEST);
	}

	// draws a single quad over the entire viewport
	void drawQuad()
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



	Pipeline* GetPipeline()
	{
		return &pipeline;
	}
}