#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/MeshUtils.h>
#include <CoreEngine/Material.h>
#include <CoreEngine/Physics.h>

class Camera;
struct MeshHandle;

namespace Components
{
  /// <summary>
  /// General components
  /// </summary>
  struct Tag
  {
    std::string tag;
  };

  struct DynamicPhysics
  {
    DynamicPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c, ::Physics::DynamicActorFlags flags = ::Physics::DynamicActorFlags{})
    {
      internalActor = ::Physics::PhysicsManager::AddDynamicActorEntity(ent, mat, c, flags);
    }
    DynamicPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c, ::Physics::DynamicActorFlags flags = ::Physics::DynamicActorFlags{})
    {
      internalActor = ::Physics::PhysicsManager::AddDynamicActorEntity(ent, mat, c, flags);
    }

    DynamicPhysics(DynamicPhysics&& rhs) noexcept { *this = std::move(rhs); }
    DynamicPhysics& operator=(DynamicPhysics&& rhs) noexcept
    {
      internalActor = std::exchange(rhs.internalActor, nullptr);
      return *this;
    }
    DynamicPhysics(const DynamicPhysics&) = delete;
    DynamicPhysics& operator=(const DynamicPhysics&) = delete;

    ::Physics::DynamicActorInterface Interface()
    {
      return internalActor;
    }

    ~DynamicPhysics()
    {
      if (internalActor)
      {
        //printf("PHYSICS %p really DELETED\n", this);
        ::Physics::PhysicsManager::RemoveActorEntity(reinterpret_cast<physx::PxRigidActor*>(internalActor));
      }
    }

  private:
    physx::PxRigidDynamic* internalActor;

  };

  struct StaticPhysics
  {
    StaticPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c)
    {
      internalActor = ::Physics::PhysicsManager::AddStaticActorEntity(ent, mat, c);
    }
    StaticPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c)
    {
      internalActor = ::Physics::PhysicsManager::AddStaticActorEntity(ent, mat, c);
    }

    StaticPhysics(StaticPhysics&& rhs) noexcept { *this = std::move(rhs); }
    StaticPhysics& operator=(StaticPhysics&& rhs) noexcept
    {
      internalActor = std::exchange(rhs.internalActor, nullptr);
      return *this;
    }
    StaticPhysics(const StaticPhysics&) = delete;
    StaticPhysics& operator=(const StaticPhysics&) = delete;

    ::Physics::StaticActorInterface Interface()
    {
      return internalActor;
    }

    ~StaticPhysics()
    {
      if (internalActor)
      {
        //printf("PHYSICS %p really DELETED\n", this);
        ::Physics::PhysicsManager::RemoveActorEntity(reinterpret_cast<physx::PxRigidActor*>(internalActor));
      }
    }

  private:
    physx::PxRigidStatic* internalActor;
  };

  struct CharacterController
  {
    //CharacterController(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c)
    //{
    //  internalController = ::Physics::PhysicsManager::AddCharacterControllerEntity(ent, mat, c);
    //}
    CharacterController(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c)
    {
      internalController = ::Physics::PhysicsManager::AddCharacterControllerEntity(ent, mat, c);
    }

    CharacterController(CharacterController&& rhs) noexcept { *this = std::move(rhs); }
    CharacterController& operator=(CharacterController&& rhs) noexcept
    {
      internalController = std::exchange(rhs.internalController, nullptr);
      return *this;
    }
    CharacterController(const CharacterController&) = delete;
    CharacterController& operator=(const CharacterController&) = delete;

    ::Physics::CharacterControllerInterface Interface()
    {
      return internalController;
    }

    ~CharacterController()
    {
      if (internalController)
      {
        //printf("PHYSICS %p really DELETED\n", this);
        ::Physics::PhysicsManager::RemoveCharacterControllerEntity(internalController);
      }
    }

  private:
    physx::PxController* internalController;
  };

  // direct member access forbidden because of isDirty flag
  struct Transform
  {
  public:
    const auto& GetTranslation() const { return translation; }
    const auto& GetRotation() const { return rotation; }
    const auto& GetScale() const { return scale; }
    const auto& GetModel() const
    {
      /*return model;*/
      return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale);
    }
    auto IsDirty() const { return isDirty; }
    void SetTranslation(const glm::vec3& vec) { translation = vec; isDirty = true; }
    void SetRotation(const glm::mat4& mat) { rotation = glm::quat_cast(mat); isDirty = true; }
    void SetRotation(const glm::quat& q) { rotation = q; isDirty = true; }
    void SetScale(const glm::vec3& vec) { scale = vec; isDirty = true; }
    //void SetModel(const glm::mat4& mat) { model = mat; isDirty = false; }
    void SetModel() { isDirty = false; }

  private:
    glm::vec3	translation{ 0 };
    //glm::mat4 rotation{ 1 };
    glm::quat rotation{ 1, 0, 0, 0 };
    glm::vec3	scale{ 1, 1, 1 };

    bool isDirty = false;
    //glm::mat4	model{ 1 };
  };

  /// <summary>
  /// Graphics components
  /// </summary>

  // temp
  //struct Mesh
  //{
  //  MeshHandle meshHandle;
  //};

  struct BatchedMesh
  {
    std::shared_ptr<MeshHandle> handle;
  };

  struct Material
  {
    std::shared_ptr<MaterialHandle> handle;
  };
  
  // TODO: temp cuz this is sucky
  struct Camera
  {
    Camera(::Camera* c) : cam(c) {}
    ::Camera* cam;
    //bool isActive;
  };

  struct Parent
  {
    Entity entity{};
  };

  struct Children
  {
    void AddChild(Entity child) 
    {
      ASSERT_MSG(std::count(children.begin(), children.end(), child) == 0, "That entity is already a child of this!");
      children.push_back(child);
    }

    void RemoveChild(Entity child)
    {
      ASSERT_MSG(std::count(children.begin(), children.end(), child) == 1, "That entity is not a child of this!");
      std::erase(children, child);
    }

    size_t size() const { return children.size(); }

    unsigned cachedHeight{};

  private:
    friend class Entity;
    std::vector<Entity> children{};
  };

  // if entity has parent, controls transform w.r.t. the parent's
  struct LocalTransform
  {
    Transform transform;
  };

  /// <summary>
  /// Scripting
  /// </summary>
  struct NativeScriptComponent
  {
    ScriptableEntity* Instance = nullptr;

    //ScriptableEntity* (*InstantiateScript)();
    std::function<ScriptableEntity* ()> InstantiateScript;
    void (*DestroyScript)(NativeScriptComponent*);

    template<typename T, typename ...Args>
    void Bind(Args&&... args)
    {
      // perfectly forwards arguments to function object with which to instantiate this script
      InstantiateScript =
        [... args = std::forward<Args>(args)] () mutable
        {
            return static_cast<ScriptableEntity*>(new T(std::forward<Args>(args)...));
        };
      DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
    }

  private:
  };
}