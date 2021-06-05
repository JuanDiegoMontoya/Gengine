#pragma once

#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

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

  virtual void OnUpdate(float dt) override
  {
    time += dt;
    auto* transform = &GetComponent<Components::LocalTransform>().transform;
    auto translation = transform->GetTranslation();
    translation.y = glm::sin(time) * 5;
    transform->SetTranslation(translation);
  }

  float time{ 0 };
};