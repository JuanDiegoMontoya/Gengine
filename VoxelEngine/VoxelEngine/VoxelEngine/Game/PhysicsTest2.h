#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Camera.h>

class PhysicsTest2 : public ScriptableEntity
{
public:
  //PhysicsTest2(Scene* scn) : scene(scn) {}

  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    // make a new physics entity
    if (Input::IsKeyPressed(GLFW_KEY_B))
    {
      //Entity entity = scene->CreateEntity("physics entity");
      //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
      ////entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
      //entity.GetComponent<Components::Transform>().SetScale({ 1, .4f, 1 });
      //entity.AddComponent<Components::BatchedMesh>().handle = meshes[0];
      //entity.AddComponent<Components::Material>(batchMaterial);
      //Components::Physics phys(entity, Physics::BoxCollider(glm::vec3(.5)));
      //entity.AddComponent<Components::Physics>(std::move(phys));
    }
    // apply some force on this entity
    if (Input::IsKeyPressed(GLFW_KEY_F))
    {
      auto dir = CameraSystem::GetDir();
      GetComponent<Components::DynamicPhysics>().Interface().AddForce(dir * 50.f);
    }
    
  }

  //Scene* scene;
};