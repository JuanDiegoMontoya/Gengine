#include <Application.h>
#include <Scene.h>
#include <Entity.h>
#include <Components.h>

#include <iostream>

void OnStart(Scene* scene)
{
  Entity thing = scene->CreateEntity("Cum");
  thing.AddComponent<Components::Transform>();
  thing.AddComponent<Components::Model>();
  thing.AddComponent<Components::Mesh>();
  thing.AddComponent<Components::Material>();
  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';
}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::Start();
  return 0;
}