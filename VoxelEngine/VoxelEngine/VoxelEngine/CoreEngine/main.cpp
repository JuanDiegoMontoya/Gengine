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
    Entity thing = scene->CreateEntity("Cum222");
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
}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::Start();
  Application::Shutdown();
  return 0;
}