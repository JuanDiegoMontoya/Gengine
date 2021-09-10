#pragma once
#include <engine/ecs/ScriptableEntity.h>
#include <engine/Input.h>
#include <engine/ecs/component/Transform.h>

class TestObj : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(Timestep timestep) override
  {
    float dt = (float)timestep.dt_effective;
    if (Input::IsKeyPressed(GLFW_KEY_K))
    {
      //auto entity = GetScene()->GetEntity("parentBigly");
      //if (entity)
      //{
      //  entity.Destroy();
      //}
      Self().Destroy();
    }

    auto* transform = &GetComponent<Component::Transform>();
    if (HasComponent<Component::LocalTransform>())
      transform = &GetComponent<Component::LocalTransform>().transform;
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