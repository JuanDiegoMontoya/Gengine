#include "gPCH.h"
#include "FlyingPlayerController.h"
#include <engine/Input.h>
#include <engine/ecs/component/Transform.h>
#include <engine/gfx/RenderView.h>
#include <engine/gfx/Camera.h>

void FlyingPlayerController::OnUpdate(Timestep timestep)
{
  glm::vec3 translation = GetComponent<Component::Transform>().GetTranslation();
  auto* camera = GetScene()->GetRenderView("main")->camera;
  auto& vi = camera->viewInfo;

  const float speed = 3.5f;

  float currSpeed = speed * timestep.dt_effective;
  if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    currSpeed *= 10;
  if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
    currSpeed /= 10;
  if (Input::IsKeyDown(GLFW_KEY_W))
    translation += currSpeed * vi.GetForwardDir();
  if (Input::IsKeyDown(GLFW_KEY_S))
    translation -= currSpeed * vi.GetForwardDir();
  if (Input::IsKeyDown(GLFW_KEY_A))
    translation -= glm::normalize(glm::cross(vi.GetForwardDir(), GFX::Constants::Up)) * currSpeed;
  if (Input::IsKeyDown(GLFW_KEY_D))
    translation += glm::normalize(glm::cross(vi.GetForwardDir(), GFX::Constants::Up)) * currSpeed;

  GetComponent<Component::Transform>().SetTranslation(translation);
  vi.position = translation;

  vi.yaw += Input::GetScreenOffset().x;
  vi.pitch = glm::clamp(vi.pitch + Input::GetScreenOffset().y, glm::radians(-89.0f), glm::radians(89.0f));
}
