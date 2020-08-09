#include "stdafx.h"
#include "chunk_manager.h"
#include "input.h"
#include "block.h"
#include "camera.h"
#include "pick.h"
#include "Editor.h"
#include "shader.h"
#include "prefab.h"

#include <fstream>
#include <functional>

#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/archives/binary.hpp>
#include <Pipeline.h>
#include "Renderer.h"
#include "ChunkStorage.h"

namespace Editor
{
	namespace RegionSelect
	{
		const int pickLength = 5;// ray cast distance
		int selectedPositions;	// how many positions have been selected
		glm::vec3 wpositions[3];	// selected positions (0-3)
		glm::vec3 hposition;			// hovered position (others are locked)
		bool open = false;
		char sName[256];
		char lName[256];
		bool skipAir = false; // if true, newly saved prefabs will skip air blocks within

		void SaveRegion()
		{
			// prefab-ify the region
			glm::vec3 min(
				glm::min(wpositions[0].x, glm::min(wpositions[1].x, wpositions[2].x)),
				glm::min(wpositions[0].y, glm::min(wpositions[1].y, wpositions[2].y)),
				glm::min(wpositions[0].z, glm::min(wpositions[1].z, wpositions[2].z)));
			glm::vec3 max(
				glm::max(wpositions[0].x, glm::max(wpositions[1].x, wpositions[2].x)),
				glm::max(wpositions[0].y, glm::max(wpositions[1].y, wpositions[2].y)),
				glm::max(wpositions[0].z, glm::max(wpositions[1].z, wpositions[2].z)));
			Prefab newPfb;
			for (int x = min.x; x <= max.x; x++)
			{
				for (int y = min.y; y <= max.y; y++)
				{
					for (int z = min.z; z <= max.z; z++)
					{
						// TODO: make bottom-middle of prefab be the origin
						Block b = ChunkStorage::AtWorldC(glm::ivec3(x, y, z));
						if (skipAir && b.GetType() == BlockType::bAir)
							continue;
						//b.SetWriteStrength(0x0F);
						newPfb.Add(
							glm::ivec3(x - min.x, y - min.y, z - min.z), b);
					}
				}
			}
			
			// append the prefab to some file
			std::ofstream os(("./resources/Prefabs/" + std::string(sName) + ".bin"), std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(newPfb);
		}

		void LoadRegion()
		{
			std::ifstream is(("./resources/Prefabs/" + std::string(lName) + ".bin").c_str(), std::ios::binary);
			cereal::BinaryInputArchive archive(is);
			Prefab oldPfb;
			archive(oldPfb);

			//WorldGen::GeneratePrefab(oldPfb, hposition);
		}

		void CancelSelection()
		{
			selectedPositions = 0;
		}

		void SelectBlock()
		{
			if (Input::Keyboard().pressed[GLFW_KEY_F])
			{
				ASSERT(selectedPositions >= 0 && selectedPositions <= 3);
				wpositions[glm::clamp(selectedPositions, 0, 3)] = hposition;
				if (selectedPositions < 3)
					selectedPositions++;
			}
		}

		void DrawSelection()
		{
			{
				ImGui::Begin("Selection Zone");
				int flag = (selectedPositions == 3 ? 0 : ImGuiButtonFlags_Disabled);
				if (ImGui::ButtonEx("save", ImVec2(0, 0), flag))
				{
					// save the prefab in a file or something fam
					SaveRegion();
				}
				ImGui::SameLine();
				ImGui::InputText("##sname", sName, 256);

				if (ImGui::Button("load"))
				{
					LoadRegion();
				}
				ImGui::SameLine();
				ImGui::InputText("##lname", lName, 256);

				ImGui::Checkbox("Skip air?", &skipAir);
				ImGui::Text("Selected positions: %d", selectedPositions);
				ImGui::Text("Hovered   : (%.2f, %.2f, %.2f)", hposition.x, hposition.y, hposition.z);
				ImGui::Text("Position 0: (%.2f, %.2f, %.2f)", wpositions[0].x, wpositions[0].y, wpositions[0].z);
				ImGui::Text("Position 1: (%.2f, %.2f, %.2f)", wpositions[1].x, wpositions[1].y, wpositions[1].z);
				ImGui::Text("Position 2: (%.2f, %.2f, %.2f)", wpositions[2].x, wpositions[2].y, wpositions[2].z);
				ImGui::End();
			}

			{
				// actually draw the bounding box
				glm::vec3 pos(0);
				glm::vec3 scale(0);
				if (selectedPositions == 0)
				{
					scale = glm::vec3(0);
					pos = hposition;
				}
				else if (selectedPositions == 1)
				{
					// component wise
					glm::vec3 min(
						glm::min(wpositions[0].x, hposition.x),
						glm::min(wpositions[0].y, hposition.y),
						glm::min(wpositions[0].z, hposition.z));
					glm::vec3 max(
						glm::max(wpositions[0].x, hposition.x),
						glm::max(wpositions[0].y, hposition.y),
						glm::max(wpositions[0].z, hposition.z));

					pos = (wpositions[0] + hposition) / 2.f;
					scale = glm::abs(max - min);
				}
				else if (selectedPositions == 2)
				{
					// component wise
					glm::vec3 min(
						glm::min(wpositions[0].x, glm::min(wpositions[1].x, hposition.x)),
						glm::min(wpositions[0].y, glm::min(wpositions[1].y, hposition.y)),
						glm::min(wpositions[0].z, glm::min(wpositions[1].z, hposition.z)));
					glm::vec3 max(
						glm::max(wpositions[0].x, glm::max(wpositions[1].x, hposition.x)),
						glm::max(wpositions[0].y, glm::max(wpositions[1].y, hposition.y)),
						glm::max(wpositions[0].z, glm::max(wpositions[1].z, hposition.z)));

					pos = (min + max) / 2.f;
					scale = glm::abs(max - min);
				}
				else// if (selectedPositions == 3)
				{
					glm::vec3 min(
						glm::min(wpositions[0].x, glm::min(wpositions[1].x, wpositions[2].x)),
						glm::min(wpositions[0].y, glm::min(wpositions[1].y, wpositions[2].y)),
						glm::min(wpositions[0].z, glm::min(wpositions[1].z, wpositions[2].z)));
					glm::vec3 max(
						glm::max(wpositions[0].x, glm::max(wpositions[1].x, wpositions[2].x)),
						glm::max(wpositions[0].y, glm::max(wpositions[1].y, wpositions[2].y)),
						glm::max(wpositions[0].z, glm::max(wpositions[1].z, wpositions[2].z)));

					pos = (min + max) / 2.f;
					scale = glm::abs(max - min);
				}

				glm::mat4 tPos = glm::translate(glm::mat4(1), pos + .5f);
				glm::mat4 tScale = glm::scale(glm::mat4(1), scale + 1.f);

				GLint polygonMode;
				glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				ShaderPtr curr = Shader::shaders["flat_color"];
				curr->Use();
				curr->setMat4("u_model", tPos * tScale);
				curr->setMat4("u_view", Renderer::GetPipeline()->GetCamera(0)->GetView());
				curr->setMat4("u_proj", Renderer::GetPipeline()->GetCamera(0)->GetProj());
				curr->setVec4("u_color", glm::vec4(1.f, .3f, 1.f, 1.f));
				Renderer::DrawCube();
				glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
				glEnable(GL_CULL_FACE);
			}
		}

		void Update()
		{
			if (Input::Keyboard().pressed[GLFW_KEY_TAB])
				open = !open;
			if (open)
			{
				raycast(
					Renderer::GetPipeline()->GetCamera(0)->GetPos(),
					Renderer::GetPipeline()->GetCamera(0)->front,
					pickLength,
					[&](glm::vec3 pos, Block block, glm::vec3 side)->bool
				{
					if (block.GetType() == BlockType::bAir)
						return false;
					if (selectedPositions == 0)
						hposition = pos;
					else if (selectedPositions == 1)
					{
						// find axis that has smallest difference, and lock that one
						glm::vec3 diff = pos - wpositions[0];
						diff = glm::abs(diff);
						float smol = std::min(diff.x, std::min(diff.y, diff.z));
						if (smol == diff.x)
							hposition = glm::vec3(wpositions[0].x, pos.y, pos.z);
						else if (smol == diff.y)
							hposition = glm::vec3(pos.x, wpositions[0].y, pos.z);
						else if (smol == diff.z)
							hposition = glm::vec3(pos.x, pos.y, wpositions[0].z);
					}
					else if (selectedPositions == 2)
					{
						// only move the axis that is shared between first two positions
						glm::vec3 diff = wpositions[1] - wpositions[0];
						hposition = wpositions[0];
						if (!diff.x)
							hposition.x = pos.x;
						if (!diff.y)
							hposition.y = pos.y;
						if (!diff.z)
							hposition.z = pos.z;
					}
					SelectBlock();
					return true;
				});

				DrawSelection();
			}
			else
			{
				CancelSelection();
			}
		}
	}

	void Update()
	{
    PERF_BENCHMARK_START;
		RegionSelect::Update();
    PERF_BENCHMARK_END;
	}
}