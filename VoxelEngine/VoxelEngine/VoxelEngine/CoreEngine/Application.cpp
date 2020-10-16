#include "Application.h"
#include "Engine.h"
#include "Scene.h"

void Application::Start()
{
  ASSERT(start);
  engine_ = new Engine();
  engine_->AddScene(new Scene("default scene", *engine_));
  start(engine_->GetScene("default scene"));
  engine_->Run();
}

void Application::Quit()
{
  engine_->Stop();
  delete engine_;
}
