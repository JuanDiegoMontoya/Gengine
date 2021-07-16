#pragma once
#include <engine/ecs/ScriptableEntity.h>
#include <engine/Input.h>
#include <engine/Application.h>

class GameManager : public ScriptableEntity
{
public:
  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate([[maybe_unused]] Timestep timestep) override
  {
    if (Input::IsKeyDown(GLFW_KEY_ESCAPE))
      Application::Quit();
  }
};