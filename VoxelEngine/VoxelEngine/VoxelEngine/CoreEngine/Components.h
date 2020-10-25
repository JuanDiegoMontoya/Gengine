#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/MeshUtils.h>
#include <CoreEngine/Material.h>

class Camera;

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
    auto IsDirty() const { return isDirty; }
    void SetTranslation(const glm::vec3& vec) { translation = vec; isDirty = true; }
    void SetRotation(const glm::mat4& mat) { rotation = mat; isDirty = true; }
    void SetScale(const glm::vec3& vec) { scale = vec; isDirty = true; }
    void SetModel(const glm::mat4& mat) { model = mat; isDirty = false; }

  private:
    glm::vec3	translation{ 0 };
    glm::mat4 rotation{ 1 };
    glm::vec3	scale{ 1, 1, 1 };

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

    ScriptableEntity* (*InstantiateScript)();
    void (*DestroyScript)(NativeScriptComponent*);

    template<typename T>
    void Bind()
    {
      InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
      DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
    }
  };
}