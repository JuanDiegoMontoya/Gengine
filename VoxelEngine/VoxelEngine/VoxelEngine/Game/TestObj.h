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
    auto* transform = &GetComponent<Components::Transform>();
    if (HasComponent<Components::LocalTransform>())
      transform = &GetComponent<Components::LocalTransform>().transform;
    if (Input::IsKeyDown(GLFW_KEY_Q))
      transform->SetTranslation(transform->GetTranslation() + dt);
    if (Input::IsKeyDown(GLFW_KEY_E))
      transform->SetTranslation(transform->GetTranslation() - dt);
    if (Input::IsKeyDown(GLFW_KEY_Z))
      transform->SetScale(transform->GetScale() + dt);
    if (Input::IsKeyDown(GLFW_KEY_X))
      transform->SetScale(transform->GetScale() - dt);

    if (Input::IsKeyDown(GLFW_KEY_C) || Input::IsKeyDown(GLFW_KEY_V))
    {
      glm::quat rot = glm::rotate(glm::quat(1, 0, 0, 0), dt, { 0, 1, 0 });
      glm::quat nrot = glm::rotate(glm::quat(1, 0, 0, 0), -dt, { 0, 1, 0 });
      if (Input::IsKeyDown(GLFW_KEY_C))
        transform->SetRotation(transform->GetRotation() * rot);
      if (Input::IsKeyDown(GLFW_KEY_V))
        transform->SetRotation(transform->GetRotation() * nrot);
    }
  }
};