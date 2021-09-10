#pragma once
#include <engine/ecs/ScriptableEntity.h>
#include <engine/Input.h>

class TestObj2 : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {
    time += rand() % 5000;
  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(Timestep timestep) override
  {
    time += timestep.dt_effective;
    auto* transform = &GetComponent<Component::LocalTransform>().transform;
    auto translation = transform->GetTranslation();
    translation.y = glm::sin(time) * 5;
    transform->SetTranslation(translation);
  }

  float time{ 0 };
};