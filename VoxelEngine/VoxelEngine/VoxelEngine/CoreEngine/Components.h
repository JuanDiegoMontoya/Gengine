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
    auto GetTranslation() const { return translation; }
    auto GetRotation() const { return rotation; }
    auto GetScale() const { return scale; }
    auto GetModel() const { return model; }
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

  // temp
  using Material = ::MaterialHandle;
  
  // TODO: temp cuz this is sucky
  struct Camera
  {
    Camera(::Camera* c) : cam(c) {}
    ::Camera* cam;
    //bool isActive;
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