#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "ScriptableEntity.h"

struct MeshHandle;

/// <summary>
/// General components
/// </summary>
struct Tag
{
  std::string tag;
};

struct Transform
{
  glm::vec3 translation;
  glm::vec3 scale;
  glm::quat rotation;
};

struct Physics
{
  glm::vec3 velocity;
  glm::vec3 acceleration;
};


/// <summary>
/// Graphics components
/// </summary>
struct Model
{
  glm::mat4 model;
  bool isDrawn;
};

// temp
struct Mesh
{
  MeshHandle meshHandle;
};

// temp
struct Material
{
  uint32_t texHandle;
};

struct Camera
{
  glm::mat4 transform;
  bool isActive;
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