#include "Application.h"
#include "Engine.h"

void Application::Start()
{
  engine_ = new Engine();
  engine_->Run();
}

void Application::Quit()
{
  engine_->Stop();
  delete engine_;
}
