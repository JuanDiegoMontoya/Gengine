#include <Application.h>
#include <Scene.h>
#include <Entity.h>
#include <Components.h>
#include "Renderer.h"
#include "Mesh.h"
#include "Material.h"

#include <iostream>

// main.cpp: this is where the user's code belongs
static MaterialHandle userMaterial{};

void OnStart(Scene* scene)
{
  MaterialInfo info;
  info.shaderID = "ShaderMcShaderFuckFace";
  info.tex2Dpaths.push_back("this is an invalid texture! (it should use a fallback)");
  userMaterial = MaterialManager::CreateMaterial(info);

  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';

  {
    Entity thing = scene->CreateEntity("bun");
    thing.AddComponent<Components::Transform>();

    thing.AddComponent<Components::Model>();
    thing.GetComponent<Components::Model>().model = glm::mat4(1.0f);

    bool l, o;
    Components::Mesh mesh;
    mesh.meshHandle = MeshManager::CreateMesh("./Resources/Models/bunny.obj", l, o)[0];
    thing.AddComponent<Components::Mesh>(mesh);

    Components::Material material = userMaterial;
    thing.AddComponent<Components::Material>(material);
    //thing.GetComponent<Components::Material>().texHandle = MeshManager::GetFuckingTexture("");
  }

  // make an entity for each object in the maya mesh
  bool l, o;
  auto mesh = MeshManager::CreateMesh("./Resources/Models/maya.fbx", l, o);
  for (auto handle : mesh)
  {
    Entity newEnt = scene->CreateEntity("maya");
    newEnt.AddComponent<Components::Transform>();
    Components::Model model{ .model = glm::scale(glm::mat4(1), {.01, .01, .01}) };
    newEnt.AddComponent<Components::Model>(model);
    Components::Mesh mesh{ .meshHandle = handle };
    newEnt.AddComponent<Components::Mesh>(mesh);
    newEnt.AddComponent<Components::Material>(userMaterial);
  }
}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::Start();
  Application::Shutdown();
  return 0;
}