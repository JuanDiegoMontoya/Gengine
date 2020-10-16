#include <Application.h>
#include <Scene.h>
#include <Entity.h>

void OnStart(Scene* scene)
{

}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::Start();
  return 0;
}