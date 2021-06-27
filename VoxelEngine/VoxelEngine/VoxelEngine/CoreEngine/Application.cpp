#include "PCH.h"
#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "GAssert.h"

#include <CoreEngine/CVar.h>
#include <CoreEngine/Parser.h>
#include <CoreEngine/Console.h>

void callback(const char* name, double value)
{
  printf("float cvar \"%s\" changed to %f", name, value);
}

void TestParse(const char* str)
{
  CmdParser parser(str);

  printf("Testing string: %s\n", str);
  while (parser.Valid())
  {
    auto var = parser.NextAtom();
    if (auto* p = std::get_if<ParseError>(&var))
    {
      printf("Parse error on char %llu: %s\n", p->where, p->what.c_str());
    }
    else if (auto* i = std::get_if<Identifier>(&var))
    {
      printf("Identifier: %s\n", i->name.c_str());
    }
    else if (auto* f = std::get_if<cvar_float>(&var))
    {
      printf("Float: %f\n", *f);
    }
    else if (auto* s = std::get_if<std::string>(&var))
    {
      printf("String: %s\n", s->c_str());
    }
    else if (auto* v = std::get_if<glm::vec3>(&var))
    {
      printf("Vector: { %f, %f, %f }\n", v->x, v->y, v->z);
    }
  }
}

AutoCVar<cvar_float> testCVarFloat("cl_testfloat", "desc", 100, CVarFlag::READ_ONLY | CVarFlag::ARCHIVE, callback);
AutoCVar<cvar_string> testCVarStr("stringy", "desc", "hallo");

void Application::Start()
{
  CVarSystem::Get()->RegisterCVar<cvar_float>("test name", "test description", 42.2);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<cvar_float>("test name"));

  CVarSystem::Get()->SetCVar<cvar_float>("test name", 2);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<cvar_float>("test name"));

  OnChangeCallback<cvar_float> cb;
  cb = callback;
  CVarSystem::Get()->RegisterCVar<cvar_float>("test name2", "test description", 5.5, CVarFlag::NONE, callback);
  CVarSystem::Get()->SetCVar<cvar_float>("test name2", 3.0);
  printf("value: %f\n", CVarSystem::Get()->GetCVar<cvar_float>("test name2"));
  printf("value: %f\n", CVarSystem::Get()->GetCVar<cvar_float>("test name"));

  CVarSystem::Get()->RegisterCVar<cvar_string>("string cvar", "string desc", "hello");
  printf("value: %s\n", CVarSystem::Get()->GetCVar<cvar_string>("string cvar"));
  CVarSystem::Get()->SetCVar<cvar_string>("string cvar", "goodbye");
  printf("value: %s\n", CVarSystem::Get()->GetCVar<cvar_string>("string cvar"));


  printf("value: %f\n", testCVarFloat.Get());
  testCVarFloat.Set(101.0101);
  printf("value: %f\n", testCVarFloat.Get());

  printf("value: %s\n", testCVarStr.Get());
  testCVarStr.Set("aloha");
  printf("value: %s\n", testCVarStr.Get());

  TestParse("__hello__ \"world\" I want 24.2 (2 3 4) bananas %%4");
  TestParse("{34.30 3 3 0 )");
  TestParse("{1.123   4 5]");
  TestParse("{1.123 a  5]");

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