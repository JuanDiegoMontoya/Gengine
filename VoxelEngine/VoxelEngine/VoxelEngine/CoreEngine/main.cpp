#include <Application.h>
#include <Scene.h>
#include <Entity.h>
#include <Components.h>
#include "Renderer.h"
#include "Mesh.h"

#include <iostream>

void OnStart(Scene* scene)
{
  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';

  {
    Entity thing = scene->CreateEntity("Cum222");
    thing.AddComponent<Components::Transform>();

    thing.AddComponent<Components::Model>();
    thing.GetComponent<Components::Model>().model = glm::mat4(1.0f);

    bool l, o;
    Components::Mesh mesh;
    mesh.meshHandle = MeshManager::CreateMesh("./Resources/Models/bunny.obj", l, o)[0];
    thing.AddComponent<Components::Mesh>(mesh);

    thing.AddComponent<Components::Material>();
    thing.GetComponent<Components::Material>().texHandle = MeshManager::GetFuckingTexture("");
  }
}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::Start();
  Application::Shutdown();
  return 0;
}