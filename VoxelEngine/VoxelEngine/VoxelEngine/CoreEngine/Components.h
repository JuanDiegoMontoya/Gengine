#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/MeshUtils.h>
#include <CoreEngine/Material.h>
#include <CoreEngine/Physics.h>

// oawie
#include <CoreEngine/TextureCube.h>

struct MeshHandle;

namespace GFX
{
  class Texture2D;
  class TextureCube;
  class StaticBuffer;
}

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
    auto GetModel() const
    {
      /*return model;*/
      return glm::translate(glm::mat4(1), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1), scale);
    }

    const auto& GetForward() const { return (glm::vec3(glm::mat4_cast(rotation)[2])); }
    const auto& GetUp() const { return (glm::vec3(glm::mat4_cast(rotation)[1])); }
    const auto& GetRight() const { return (glm::vec3(glm::mat4_cast(rotation)[0])); }

    auto IsDirty() const { return isDirty; }
    void SetTranslation(const glm::vec3& vec) { translation = vec; isDirty = true; }
    void SetRotation(const glm::mat4& mat) { rotation = glm::quat_cast(mat); isDirty = true; }
    void SetRotation(const glm::quat& q) { rotation = q; isDirty = true; }
    void SetScale(const glm::vec3& vec) { scale = vec; isDirty = true; }
    //void SetModel(const glm::mat4& mat) { model = mat; isDirty = false; }
    void SetModel() { isDirty = false; }

    void SetPYR(const glm::vec3& pyr) { pitch_ = pyr.x; yaw_ = pyr.y; roll_ = pyr.z; isDirty = true; }

    // Temp
    float pitch_ = 16;
    float yaw_ = 255;
    float roll_ = 0;

  private:
    glm::vec3	translation{ 0 };
    //glm::mat4 rotation{ 1 };
    glm::quat rotation{ 1, 0, 0, 0 };
    glm::vec3	scale{ 1, 1, 1 };

    glm::vec3 forward{ 0, 0, 1 };
    glm::vec3 up{ 0, 0, 1 };
    glm::vec3 right{ 0, 0, 1 };

    bool isDirty = false;
    //glm::mat4	model{ 1 };
  };

  /// <summary>
  /// Graphics components
  /// </summary>

  struct BatchedMesh
  {
    std::shared_ptr<MeshHandle> handle;
    uint64_t renderFlag = (uint64_t)RenderFlags::Default;
  };

  struct Material
  {
    std::shared_ptr<MaterialHandle> handle;
  };

  struct Camera
  {
    Camera(Entity);

    void AddCullingFlag(RenderFlags renderFlag)
    {
      cullingMask |= (uint64_t)renderFlag;
    }

    void RemoveCullingFlag(RenderFlags renderFlag)
    {
      cullingMask &= ~(uint64_t)renderFlag;
    }

    void SetPos(glm::vec3 pos) { translation = pos; }

    void SetYaw(float f) { yaw_ = f; }
    void SetPitch(float f) { pitch_ = f; }
    void SetDir(glm::vec3 d) { dir = d; }

    glm::vec3 GetEuler() { return { pitch_, yaw_, roll_ }; }

    const auto& GetWorldPos()
    {
      Transform& tr = entity.GetComponent<Components::Transform>();
      return tr.GetTranslation() + translation;
    }

    const auto& GetLocalPos() { return translation; }

    const auto& GetForward()
    {
      dir.x = cos(glm::radians(pitch_)) * cos(glm::radians(yaw_));
      dir.y = sin(glm::radians(pitch_));
      dir.z = cos(glm::radians(pitch_)) * sin(glm::radians(yaw_));
      return dir;
    }

    glm::vec3	dir{ 0 };
    glm::vec3	translation{ 0 };

    float pitch_ = 16;
    float yaw_ = 255;
    float roll_ = 0;

    Entity entity{};

    bool frustumCulling = false;

    uint64_t cullingMask = (uint64_t)RenderFlags::NoRender;

    float fov = 70.0f;

    float zNear = 0.1f;
    float zFar = 1000.0f;

    GFX::TextureCube* skybox{};
    GLuint renderTexture = 0;
  };

  struct ParticleEmitter
  {
    ParticleEmitter(uint32_t maxp, std::string tex);
    ParticleEmitter(ParticleEmitter&& rhs) noexcept;
    ParticleEmitter& operator=(ParticleEmitter&& rhs) noexcept;
    ~ParticleEmitter() = default;
    ParticleEmitter(const ParticleEmitter&) = delete;
    ParticleEmitter& operator=(const ParticleEmitter&) = delete;

    // initial particle offset
    glm::vec3 minParticleOffset{ -1 };
    glm::vec3 maxParticleOffset{ 1 };
    glm::vec3 minParticleVelocity{ -1 };
    glm::vec3 maxParticleVelocity{ 1 };
    glm::vec3 minParticleAccel{ -1 };
    glm::vec3 maxParticleAccel{ 1 };
    glm::vec2 minParticleScale{ .1 };
    glm::vec2 maxParticleScale{ 1 };
    glm::vec4 minParticleColor{ 1 };
    glm::vec4 maxParticleColor{ 1 };

    float timer{ 0.0f };
    float interval{ 0.1f };
    float minLife{ 1 };
    float maxLife{ 1 };

    uint32_t numParticles{ 0 };
    const uint32_t maxParticles; // const

    uint64_t renderFlag = (uint64_t)RenderFlags::Default;

  private:
    friend class Renderer;
    friend class ParticleSystem;

    std::unique_ptr<GFX::StaticBuffer> particleBuffer{};
    std::unique_ptr<GFX::StaticBuffer> freeStackBuffer{};
    std::unique_ptr<GFX::Texture2D> texture{};
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
        [... args = std::forward<Args>(args)]() mutable
      {
        return static_cast<ScriptableEntity*>(new T(std::forward<Args>(args)...));
      };
      DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
    }
  };
}