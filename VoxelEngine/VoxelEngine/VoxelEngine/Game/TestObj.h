#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

class TestObj : public ScriptableEntity
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
    auto& transform = GetComponent<Components::Transform>();
    if (Input::IsKeyDown(GLFW_KEY_Q))
      transform.SetTranslation(transform.GetTranslation() + dt);
    if (Input::IsKeyDown(GLFW_KEY_E))
      transform.SetTranslation(transform.GetTranslation() - dt);
    if (Input::IsKeyDown(GLFW_KEY_Z))
      transform.SetScale(transform.GetScale() + dt);
    if (Input::IsKeyDown(GLFW_KEY_X))
      transform.SetScale(transform.GetScale() - dt);

    glm::mat4 rot = glm::rotate(glm::mat4(1), dt, { 0, 1, 0 });
    glm::mat4 nrot = glm::rotate(glm::mat4(1), -dt, { 0, 1, 0 });
    if (Input::IsKeyDown(GLFW_KEY_C))
      transform.SetRotation(transform.GetRotation() * rot);
    if (Input::IsKeyDown(GLFW_KEY_V))
      transform.SetRotation(transform.GetRotation() * nrot);
  }
};