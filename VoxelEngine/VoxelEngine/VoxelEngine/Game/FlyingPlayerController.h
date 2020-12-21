#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

class FlyingPlayerController : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    auto translation = GetComponent<Components::Transform>().GetTranslation();
    auto& cam = *CameraSystem::ActiveCamera;

    float speed = 3.5f;

    float currSpeed = speed * dt;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
      currSpeed *= 10;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
      currSpeed /= 10;
    if (Input::IsKeyDown(GLFW_KEY_W))
      translation += currSpeed * CameraSystem::GetFront();
    if (Input::IsKeyDown(GLFW_KEY_S))
      translation -= currSpeed * CameraSystem::GetFront();
    if (Input::IsKeyDown(GLFW_KEY_A))
      translation -= glm::normalize(glm::cross(CameraSystem::GetFront(), CameraSystem::GetUp())) * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_D))
      translation += glm::normalize(glm::cross(CameraSystem::GetFront(), CameraSystem::GetUp())) * currSpeed;
    GetComponent<Components::Transform>().SetTranslation(translation);
    //CameraSystem::SetPos(translation); // TODO: TEMP BULLSHIT
    //
    auto pyr = CameraSystem::GetEuler();
    CameraSystem::SetYaw(pyr.y + Input::GetScreenOffset().x);
    CameraSystem::SetPitch(glm::clamp(pyr.x + Input::GetScreenOffset().y, -89.0f, 89.0f));
    //pyr = CameraSystem::GetEuler(); // oof
    //
    //glm::vec3 temp;
    //temp.x = cos(glm::radians(pyr.x)) * cos(glm::radians(pyr.y));
    //temp.y = sin(glm::radians(pyr.x));
    //temp.z = cos(glm::radians(pyr.x)) * sin(glm::radians(pyr.y));
    //CameraSystem::SetFront(glm::normalize(temp));
    //CameraSystem::SetDir(CameraSystem::GetFront());
  }
};