#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

class PlayerController : public ScriptableEntity
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
    // TODO: put in game manager script
    if (Input::IsKeyDown(GLFW_KEY_ESCAPE))
      Application::Quit();

    auto translation = GetComponent<Components::Transform>().GetTranslation();
    auto& cam = *GetComponent<Components::Camera>().cam;

    float currSpeed = speed * dt;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
      currSpeed *= 10;
    if (Input::IsKeyDown(GLFW_KEY_LEFT_CONTROL))
      currSpeed /= 10;
    if (Input::IsKeyDown(GLFW_KEY_W))
      translation += currSpeed * cam.GetFront();
    if (Input::IsKeyDown(GLFW_KEY_S))
      translation -= currSpeed * cam.GetFront();
    if (Input::IsKeyDown(GLFW_KEY_A))
      translation -= glm::normalize(glm::cross(cam.GetFront(), cam.GetUp())) * currSpeed;
    if (Input::IsKeyDown(GLFW_KEY_D))
      translation += glm::normalize(glm::cross(cam.GetFront(), cam.GetUp())) * currSpeed;
    GetComponent<Components::Transform>().SetTranslation(translation);
    cam.SetPos(translation); // TODO: TEMP BULLSHIT

    auto pyr = cam.GetEuler();
    cam.SetYaw(pyr.y + Input::GetScreenOffset().x);
    cam.SetPitch(glm::clamp(pyr.x + Input::GetScreenOffset().y, -89.0f, 89.0f));
    pyr = cam.GetEuler(); // oof

    glm::vec3 temp;
    temp.x = cos(glm::radians(pyr.x)) * cos(glm::radians(pyr.y));
    temp.y = sin(glm::radians(pyr.x));
    temp.z = cos(glm::radians(pyr.x)) * sin(glm::radians(pyr.y));
    cam.SetFront(glm::normalize(temp));
    cam.SetDir(cam.GetFront());
  }

  float speed = 3.5f;
};