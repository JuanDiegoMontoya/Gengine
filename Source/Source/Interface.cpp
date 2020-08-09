#include "stdafx.h"
#include "Interface.h"

#include <shader.h>
#include "World.h"
#include "Renderer.h"
#include <Pipeline.h>
#include <camera.h>
#include <Engine.h>
#include "pick.h"
#include "settings.h"
#include "ImGuiBonus.h"
#include <input.h>
#include "ChunkStorage.h"
#include "ChunkHelpers.h"
#include "ChunkMesh.h"
#include "ChunkRenderer.h"
#include "sun.h"
#include "RenderOrder.h"

#include <zlib.h>

namespace Interface
{
#pragma optimize("", off)
	void Init()
	{
		Interface::activeCursor = false;
		glfwSetInputMode(Engine::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Engine::PushRenderCallback(DrawImGui, RenderOrder::RenderInterfaceImGui);
		Engine::PushRenderCallback(Update, RenderOrder::RenderInterfaceUpdate);
	}
#pragma optimize("", on)

	void Update()
	{
		if (Input::Keyboard().pressed[GLFW_KEY_GRAVE_ACCENT])
		{
			Interface::activeCursor = !Interface::activeCursor;
		}

		if (Interface::activeCursor)
		{
			//glfwSetInputMode(Engine::GetWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			glfwSetInputMode(Engine::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			//glfwSetInputMode(Engine::GetWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
			glfwSetInputMode(Engine::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}

	void DrawImGui()
	{
		PERF_BENCHMARK_START;

		static bool renderUI = true;
		ImGui::SetNextWindowBgAlpha(.5f);
		{
			ImGui::Begin("##joe");
			ImGui::Checkbox("Show UI", &renderUI);
			ImGui::Checkbox("Show Plot", &debug_graphs);
			ImGui::End();
		}

		if (renderUI)
		{
			// chunk info
			ImGui::SetNextWindowBgAlpha(.5f);
			{
				ImGui::Begin("Chunk Info", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);

				ImGui::Text("Chunk size: %d", Chunk::CHUNK_SIZE);

				// displaying zero just means the queue was taken, not finished!
				ImGui::Text("Gen queue:    %d", World::chunkManager_.generation_queue_.size());
				ImGui::Text("Mesh queue:   %-4d (%d)", World::chunkManager_.mesher_queue_.size(), World::chunkManager_.debug_cur_pool_left.load());
				ImGui::Text("Buffer queue: %d", World::chunkManager_.buffer_queue_.size());

				static bool countChunks = true;
				ImGui::Checkbox("Count chunks (slow)", &countChunks);
				if (countChunks)
				{
					int nonNull = 0;
					int active = 0;
					int numVerts = 0;
					int numPoints = 0;
					// this causes lag with many chunks
					for (auto& p : ChunkStorage::GetMapRaw())
					{
						if (p.second)
						{
							nonNull++;
							numVerts += p.second->GetMesh().GetVertexCount();
							numPoints += p.second->GetMesh().GetPointCount();
						}
					}
					ImGui::Text("Total chunks:    %d", ChunkStorage::GetMapRaw().size());
					ImGui::Text("Non-null chunks: %d", nonNull);
					ImGui::Text("Drawn chunks:    %d", NuRenderer::drawCalls);
					ImGui::Text("Culled chunks:   %d", nonNull - NuRenderer::drawCalls);

					ImGui::NewLine();
					ImGui::Text("Vertices: %d", numVerts);
					ImGui::Text("Points:   %d", numPoints);
					ImGui::NewLine();
				}
				
				ImGui::End();
			}


			ImGui::SetNextWindowBgAlpha(.5f);
			{
				ImGui::Begin("Sun", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
				ImGuiWindowFlags f;
				glm::vec3 pos = World::sun_->GetPos();
				if (ImGui::DragFloat3("Sun Pos", &pos[0], 1, -500, 500, "%.0f"))
					World::sun_->SetPos(pos);

				glm::vec3 dir = World::sun_->GetDir();
				if (ImGui::SliderFloat3("Sun Dir", &dir[0], -1, 1, "%.3f"))
					World::sun_->SetDir(dir);

				ImGui::Checkbox("Orbit Pos", &World::sun_->orbits);
				ImGui::SameLine();
				ImGui::DragFloat3("##Orbitee", &World::sun_->orbitPos[0], 2.f, -500, 500, "%.0f");
				ImGui::Checkbox("Follow Cam", &World::sun_->followCam);
				ImGui::SliderFloat("Follow Distance", &World::sun_->followDist, 0, 500, "%.0f");
				ImGui::Checkbox("Collision Enabled", &World::doCollisionTick);

				bool val = Renderer::GetPipeline()->GetCamera(0)->GetType() == CameraType::kPhysicsCam;
				if (ImGui::Checkbox("Camera Has Gravity", &val))
				{
					Renderer::GetPipeline()->GetCamera(0)->SetType(val ? CameraType::kPhysicsCam : CameraType::kControlCam);
				}

				//int shadow = World::sun_->GetShadowSize().x;
				//if (ImGui::InputInt("Shadow Scale", &shadow, 1024, 1024))
				//{
				//	glm::clamp(shadow, 0, 16384);
				//	World::sun_->SetShadowSize(glm::ivec2(shadow));
				//}
				//if (ImGui::Button("Recompile Water Shader"))
				//{
				//	delete Shader::shaders["chunk_water"];
				//	Shader::shaders["chunk_water"] = new Shader("chunk_water.vs", "chunk_water.fs");
				//}
				//if (ImGui::Button("Recompile Debug Map"))
				//{
				//	delete Shader::shaders["debug_map3"];
				//	Shader::shaders["debug_map3"] = new Shader("debug_map.vs", "debug_map.fs");
				//}
				//if (ImGui::Button("Recompile Postprocess Shader"))
				//{
				//	//delete Shader::shaders["postprocess"];
				//	Shader::shaders["postprocess"] = new Shader("postprocess.vs", "postprocess.fs");
				//}
				if (ImGui::Button("Recompile Optimized Chunk Shader"))
				{
					delete Shader::shaders["chunk_optimized"];
					Shader::shaders["chunk_optimized"] = new Shader("chunk_optimized.vs", "chunk_optimized.fs");
				}
				if (ImGui::Button("Recompile Splat Chunk Shader"))
				{
					delete Shader::shaders["chunk_splat"];
					Shader::shaders["chunk_splat"] = new Shader("chunk_splat.vs", "chunk_splat.fs");
				}
				if (ImGui::Button("Recompile Render Cull Shader"))
				{
					delete Shader::shaders["chunk_render_cull"];
					Shader::shaders["chunk_render_cull"] = new Shader("chunk_render_cull.vs", "chunk_render_cull.fs");
				}

				if (ImGui::Button("Delete far chunks (unsafe)"))
				{
					std::vector<ChunkPtr> deleteList;
					std::for_each(ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end(),
						[&](auto& p)
					{
						float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
						if (p.second && dist > World::chunkManager_.loadDistance_ + World::chunkManager_.unloadLeniency_)
						{
							deleteList.push_back(p.second);
							p.second = nullptr;
						}
					});

					for (ChunkPtr p : deleteList)
						delete p;
				}

				static char fileName[256];
				ImGui::InputText("Map path", fileName, 256u);
				if (ImGui::Button("Save Map"))
				{
					World::chunkManager_.SaveWorld(fileName);
				}
				if (ImGui::Button("Load Map"))
				{
					World::chunkManager_.LoadWorld(fileName);
				}

				ImGui::End();
			}

			// big thing
			ImGui::SetNextWindowBgAlpha(.5f);
			{
				static int fakeLag = 0;
				static double frameTimeExp = 0;
				static double alpha = .01;

				frameTimeExp = alpha * Engine::GetDT() + (1.0 - alpha) * frameTimeExp;
				alpha = glm::clamp((float)Engine::GetDT(), 0.0f, 1.0f);

				ImGui::Begin("Info", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
				ImGui::Text("FPS: %.0f (%.1f ms)", 1.f / frameTimeExp, frameTimeExp * 1000);
				
				ImGui::SliderInt("Fake Lag", &fakeLag, 0, 50, "%d ms");
				if (fakeLag > 0)
					std::this_thread::sleep_for(milliseconds(fakeLag));

				ImGui::NewLine();
				glm::vec3 pos = Renderer::GetPipeline()->GetCamera(0)->GetPos();
				//ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
				ImGui::SliderFloat("Render distance", &World::chunkManager_.loadDistance_, 0, 5000, "%.0f");
				ImGui::SliderFloat("Leniency distance", &World::chunkManager_.unloadLeniency_, 0, 1000, "%.0f");
				float ffar = Renderer::GetPipeline()->GetCamera(0)->GetFar();
				if (ImGui::SliderFloat("Far plane", &ffar, 0.1f, 5000, "%.0f"))
					Renderer::GetPipeline()->GetCamera(0)->SetFar(ffar);
				if (ImGui::InputFloat3("Camera Position", &pos[0], 2))
					Renderer::GetPipeline()->GetCamera(0)->SetPos(pos);
				pos = Renderer::GetPipeline()->GetCamera(0)->front;
				ImGui::Text("Camera Direction: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
				pos = pos * .5f + .5f;
				ImGui::SameLine();
				ImGui::ColorButton("visualization", ImVec4(pos.x, pos.y, pos.z, 1.f));

				ChunkHelpers::localpos local = ChunkHelpers::worldPosToLocalPos(Renderer::GetPipeline()->GetCamera(0)->GetPos());
				ImGui::Text("In chunk pos: (%d, %d, %d)", local.chunk_pos.x, local.chunk_pos.y, local.chunk_pos.z);
				ImGui::Text("In block pos: (%d, %d, %d)", local.block_pos.x, local.block_pos.y, local.block_pos.z);

				ImGui::NewLine();

				ImGui::NewLine();
				ImGui::Text("Flying: %s", activeCursor ? "False" : "True");

				const glm::vec3 camPos = Renderer::GetPipeline()->GetCamera(0)->GetPos();

				float dist = 5.f;
				ImGui::Text("Voxel raycast information:");
				ImGui::Text("Ray length: %0.f", dist);
				raycast(
					Renderer::GetPipeline()->GetCamera(0)->GetPos(),
					Renderer::GetPipeline()->GetCamera(0)->front,
					dist,
					std::function<bool(glm::vec3, Block, glm::vec3)>
					([&](glm::vec3 pos, Block block, glm::vec3 side)->bool
				{
					if (block.GetType() == BlockType::bAir)
						return false;

					ImGui::Text("Block Type: %d (%s)", (unsigned)block.GetType(), block.GetName());
					//ImGui::Text("Write Strength: %d", block->WriteStrength());
					//ImGui::Text("Light Value: %d", block->LightValue());
					Light lit = ChunkStorage::AtWorldC(pos).GetLight();
					Light lit2 = ChunkStorage::AtWorldC(pos + side).GetLight();
					ImGui::Text("Light: (%d, %d, %d, %d)", lit.GetR(), lit.GetG(), lit.GetB(), lit.GetS());
					ImGui::Text("FLight: (%d, %d, %d, %d)", lit2.GetR(), lit2.GetG(), lit2.GetB(), lit2.GetS());
					ImGui::Text("Block pos:  (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
					ImGui::Text("Block side: (%.2f, %.2f, %.2f)", side.x, side.y, side.z);
					//glm::vec3 color = Block::PropertiesTable[block->GetType()].color;
					//ImGui::ColorPicker3("colorr", )

					ShaderPtr curr = Shader::shaders["flat_color"];
					curr->Use();
					curr->setMat4("u_model", glm::translate(glm::mat4(1), pos + .5f));
					curr->setMat4("u_view", Renderer::GetPipeline()->GetCamera(0)->GetView());
					curr->setMat4("u_proj", Renderer::GetPipeline()->GetCamera(0)->GetProj());
					curr->setVec4("u_color", glm::vec4(1, 1, 1, .4f));
					glLineWidth(2);
					GLint polygonMode;
					glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					Renderer::DrawCube();
					glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

					return true;
				}
				));

				ImGui::End();
			}

			// render
			ImGui::SetNextWindowBgAlpha(.5f);
			{
				ImGui::Begin("Render Settings", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
				if (ImGui::Checkbox("Compute baked AO", &Settings::GFX::blockAO))
					World::chunkManager_.ReloadAllChunks();
				if (ImGui::Checkbox("Skip lighting", &ChunkMesh::debug_ignore_light_level))
					World::chunkManager_.ReloadAllChunks();
				ImGui::Checkbox("Gamma correction", &NuRenderer::settings.gammaCorrection);
				ImGui::Checkbox("Freeze Culling", &ChunkRenderer::settings.freezeCulling);
				ImGui::Checkbox("Draw Occ. Culling", &ChunkRenderer::settings.debug_drawOcclusionCulling);
				ImGui::SliderFloat("Fog Start", &NuRenderer::settings.fogStart, 0, 5000);
				ImGui::SliderFloat("Fog End", &NuRenderer::settings.fogEnd, 0, 5000);

				ImGui::Text("Render distance:");
				ImGui::SliderFloat("normalMin", &ChunkRenderer::settings.normalMin, 0, 5000);
				ImGui::SliderFloat("normalMax", &ChunkRenderer::settings.normalMax, 0, 5000);
				ImGui::SliderFloat("splatMin", &ChunkRenderer::settings.splatMin, 0, 5000);
				ImGui::SliderFloat("splatMax", &ChunkRenderer::settings.splatMax, 0, 5000);
				ImGui::End();
			}

			// graphs
			ImGui::SetNextWindowBgAlpha(.5f);
			if (debug_graphs)
			{
				ImGui::Begin("Graphs", 0, activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
				ImGui::Text("Avg Mesh Time: %.3f", ChunkMesh::accumtime / ChunkMesh::accumcount);
				ImGui::PlotVar("Frametime", Engine::GetDT() * 1000.0, 0, .05 * 1000, 240, ImVec2(300, 100));

				if (Renderer::nvUsageEnabled)
				{
					GLint totalMemoryKb = 0;
					//glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);
					glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalMemoryKb);

					GLint currentMemoryKb = 0;
					glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKb);
					//ImGui::PlotVar("VRAM usage", float(totalMemoryKb - currentMemoryKb) / 1000.f, 0, totalMemoryKb / 1000, 300, ImVec2(300, 100));
					ImGui::Text("VRAM usage: %.0f / %.0f MB", float(totalMemoryKb - currentMemoryKb) / 1000.f, float(totalMemoryKb) / 1000.f);
				}
				else
					ImGui::Text("VRAM usage graph disabled due to incompatible GPU");
				ImGui::End();
			}

			// Networking interface
			{
				ImGui::Begin("Networking");
				const int bufsize = 100;
				static char address[bufsize] = "localhost";
				static int port = 1234;

				ImGui::Text("Client/Server tick: %.0f/%.0f", CLIENT_NET_TICKS_PER_SECOND, SERVER_NET_TICKS_PER_SECOND);

				ImGui::InputText("Address", address, bufsize);
				ImGui::InputInt("Port", &port, 0);
				if (!World::client.GetConnected())
					if (ImGui::Button("Connect"))
						World::client.Connect(address, port);
				if (World::client.GetConnected())
					if (ImGui::Button("Disconnect"))
						World::client.DisconnectFromCurrent();
				
				auto& playerWorld = World::client.GetPlayerWorld();
				ImGui::Text("Connected players: %d", playerWorld.GetObjects_Unsafe().size());
				ImGui::Text("My ID: %d", World::client.GetThisID());
				ImGui::Separator();
				for (const auto& [id, obj] : playerWorld.GetObjects_Unsafe())
				{
					ImGui::Text("ID: %d", id);
				}
				ImGui::End();
			}
		}

		PERF_BENCHMARK_END;
	}
}