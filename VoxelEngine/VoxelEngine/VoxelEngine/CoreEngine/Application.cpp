#include "EnginePCH.h"
#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "GAssert.h"

#include <CoreEngine/CVar.h>

void callback(const char* name, float value)
{
  printf("float cvar \"%s\" changed to %f", name, value);
}

AutoCVar<float> testCVarFloat("cl_testfloat", "desc", 100, CVarFlag::READ_ONLY | CVarFlag::ARCHIVE, callback);
AutoCVar<int> testCVarInt("integer", "desc", 50, CVarFlag::NONE);
AutoCVar<const char*> testCVarStr("stringy", "desc", "hallo");

void Application::Start()
{
  CVarSystem::Get()->RegisterCVar<float>("test name", "test description", 42);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<float>("test name"));

  CVarSystem::Get()->SetCVar<float>("test name", 2);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<float>("test name"));

  OnChangeCallback<float> cb;
  cb = callback;
  CVarSystem::Get()->RegisterCVar<float>("test name2", "test description", 5, CVarFlag::NONE, callback);
  CVarSystem::Get()->SetCVar<float>("test name2", 3);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<float>("test name2"));
  printf("value: %f\n", CVarSystem::Get()->GetCVar<float>("test name"));

  CVarSystem::Get()->RegisterCVar<const char*>("string cvar", "string desc", "hello");
  printf("value: %s\n", CVarSystem::Get()->GetCVar<const char*>("string cvar"));
  CVarSystem::Get()->SetCVar<const char*>("string cvar", "goodbye");
  printf("value: %s\n", CVarSystem::Get()->GetCVar<const char*>("string cvar"));

  CVarSystem::Get()->RegisterCVar<int>("intcvar", "desc", 12);
  printf("value: %d\n", CVarSystem::Get()->GetCVar<int>("intcvar"));
  CVarSystem::Get()->SetCVar<int>("intcvar", 21);
  printf("value: %d\n", CVarSystem::Get()->GetCVar<int>("intcvar"));


  printf("value: %f\n", testCVarFloat.Get());
  testCVarFloat.Set(101);
  printf("value: %f\n", testCVarFloat.Get());

  printf("value: %d\n", testCVarInt.Get());
  testCVarInt.Set(51);
  printf("value: %d\n", testCVarInt.Get());

  printf("value: %s\n", testCVarStr.Get());
  testCVarStr.Set("aloha");
  printf("value: %s\n", testCVarStr.Get());

  ASSERT(start);
  engine_ = new Engine();
  engine_->AddScene(new Scene("default scene", *engine_));
  engine_->SetActiveScene(0);
  engine_->InitScenes();
  start(engine_->GetScene(0));
  engine_->updateCallback = update;
  engine_->drawCallback = draw;
  engine_->Run();
}

void Application::Shutdown()
{
  delete engine_;
}

void Application::Quit()
{
  engine_->Stop();
}