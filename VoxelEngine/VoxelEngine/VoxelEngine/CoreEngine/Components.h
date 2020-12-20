#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/MeshUtils.h>
#include <CoreEngine/Material.h>

namespace Components
{
  /// <summary>
  /// General components
  /// </summary>
  struct Tag
  {
    std::string tag;
  };

  struct Physics
  {
    glm::vec3 velocity;
    glm::vec3 acceleration;
  };

  struct AABBCollider
  {
    glm::vec3 center; // relative to transform
    glm::vec3 scale;
  };

  // direct member access forbidden because of isDirty flag
  struct Transform
  {
  public:
    const auto& GetTranslation() const { return translation; }
    const auto& GetRotation() const { return rotation; }
    const auto& GetScale() const { return scale; }
    const auto& GetModel() const { return model; }

    const auto& GetForward() const { return (glm::vec3(rotation[2])); }
    const auto& GetUp() const { return (glm::vec3(rotation[1])); }
    const auto& GetRight() const { return (glm::vec3(rotation[0])); }

    auto IsDirty() const { return isDirty; }
    void SetTranslation(const glm::vec3& vec) { translation = vec; isDirty = true; }
    void SetRotation(const glm::mat4& mat) { rotation = mat; isDirty = true; }
    void SetScale(const glm::vec3& vec) { scale = vec; isDirty = true; }
    void SetModel(const glm::mat4& mat) { model = mat; isDirty = false; }

    void SetPYR(const glm::vec3& pyr) { pitch_ = pyr.x; yaw_ = pyr.y; roll_ = pyr.z; isDirty = true; }

    // Temp
    float pitch_ = 16;
    float yaw_ = 255;
    float roll_ = 0;

  private:
    glm::vec3	translation{ 0 };
    glm::mat4 rotation{ 1 };
    glm::vec3	scale{ 1, 1, 1 };

    glm::vec3 forward{ 0, 0, 1 };
    glm::vec3 up{ 0, 0, 1 };
    glm::vec3 right{ 0, 0, 1 };

    glm::mat4	model{ 1 };
    bool isDirty = false;
  };

  /// <summary>
  /// Graphics components
  /// </summary>

  // temp
  struct Mesh
  {
    MeshHandle meshHandle;
  };

  struct BatchedMesh
  {
    BatchedMeshHandle handle;
  };

  // temp
  using Material = ::MaterialHandle;
  
  struct Camera
  {
    /*Camera(::Camera* c) : cam(c) 
    {

    }
    ::Camera* cam;*/
    //bool isActive;

    // New
      Camera(Entity);

    Entity entity{};

    bool frustumCulling = false;
    // Culling Mask

    float fov = 70.0f;

    float zNear = 0.1f;
    float zFar = 1000.0f;

    unsigned skybox = 0;
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