#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Application.h>

class GameManager : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate([[maybe_unused]] float dt) override
  {
    if (Input::IsKeyDown(GLFW_KEY_ESCAPE))
      Application::Quit();
  }
};