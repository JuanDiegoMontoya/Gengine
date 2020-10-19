#include "Application.h"
#include "Engine.h"
#include "Scene.h"
#include "GAssert.h"

void Application::Start()
{
  ASSERT(start);
  engine_ = new Engine();
  engine_->AddScene(new Scene("default scene", *engine_));
  engine_->SetActiveScene(0);
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