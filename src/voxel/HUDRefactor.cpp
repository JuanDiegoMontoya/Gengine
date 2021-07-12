#include "vPCH.h"
#include <glad/glad.h>
#include <voxel/HUDRefactor.h>
#include <engine/Input.h>
#include <engine/Camera.h>
#include <engine/gfx/ShaderManager.h>
#include <engine/gfx/Renderer.h>
#include <imgui/imgui.h>


void HUD::Update()
{
  int ofs = static_cast<int>(Input::GetScrollOffset().y);
  int num = static_cast<int>(selected_) + ofs;
  //if (num >= int(BlockType::bCount) || num < 0)
  //  ofs = 0;
  if (ofs)
  {
    //printf("%d\n", selected_);
    selected_ = (Voxels::BlockType)glm::clamp(num, 0, (int)Voxels::BlockType::bCount-1);
    //printf("%d\n", selected_);
  }
  //reinterpret_cast<unsigned char&>(selected_) += ofs;

  //Camera* cam = CameraSystem::ActiveCamera;----

  //printf("%f\n", Input::Mouse().scrollOffset.y);

  // render the selected object on the screen
  glm::vec3 pos = glm::vec3(0, -6, -12);// +cam->GetPos() + cam->front * 10.f;
  glm::vec3 rot(.5, 1, 0);
  glm::vec3 scl(1, 1, 1);
  glm::mat4 model =
    glm::translate(glm::mat4(1), pos) *
    glm::rotate(glm::mat4(1), (float)glfwGetTime(), rot) *
    glm::scale(glm::mat4(1), scl);
  {
    auto curr = GFX::ShaderManager::Get()->GetShader("textured_array");
    curr->Bind();
    curr->SetMat4("u_model", model);
    curr->SetMat4("u_proj", CameraSystem::GetProj());
    curr->SetMat4("u_view", glm::mat4(1));
    //curr->setVec4("u_color", Block::PropertiesTable[int(selected_)].color);
    ASSERT_MSG(false, "Do something to make next line work");
    //Renderer::GetBlockTextures()->Bind(0);
    curr->SetInt("u_textures", 0);
    curr->SetInt("u_texIdx", int(selected_));
    //Renderer::DrawCube(); // block // TODO
  }

  {
    auto curr = GFX::ShaderManager::Get()->GetShader("flat_color");
    curr->SetVec4("u_color", glm::vec4(1));
    model = glm::scale(model, { 1.1, 1.1, 1.1 });
    curr->SetMat4("u_model", model);
    curr->SetMat4("u_proj", CameraSystem::GetProj());
    curr->SetMat4("u_view", glm::mat4(1));
  }

  GLint polygonMode;
  glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  //Renderer::DrawCube(); // wireframe cube outside of block
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

  // display name of held block
  {
    //ImGui::Begin("##held", 0, Interface::activeCursor ? 0 : ImGuiWindowFlags_NoMouseInputs);
    //ImGui::Text("Held: %s", Block::PropertiesTable[int(selected_)].name);
    //ImGui::End();
  }
}
