#pragma once
#include <Voxels/VoxelManager.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/ScriptableEntity.h>
#include <Voxels/prefab.h>

class PlayerActions : public ScriptableEntity
{
public:
  PlayerActions(VoxelManager* vm) : voxels(vm) {}

  virtual void OnCreate() override;
  virtual void OnDestroy() override;
  virtual void OnUpdate(float dt) override;

  void checkTestButton();
  void checkBlockPlacement();
  void checkBlockDestruction(float dt);
  void checkBlockPick();

  glm::vec3 prevBlock = { -1, -1, -1 };
  bool prevHit = false;
  float timer = 0.0f;
  BlockType selected = BlockType::bStone;
  VoxelManager* voxels{};
  Prefab prefab = PrefabManager::GetPrefab("Error");
  std::string prefabName = "Error";

  // particle emitter settings
  glm::vec3 minParticleOffset{ -1 };
  glm::vec3 maxParticleOffset{ 1 };
  glm::vec3 minParticleVelocity{ -1 };
  glm::vec3 maxParticleVelocity{ 1 };
  glm::vec3 minParticleAccel{ -1 };
  glm::vec3 maxParticleAccel{ 1 };
  glm::vec2 minParticleScale{ .1f };
  glm::vec2 maxParticleScale{ 1 };
  glm::vec4 minParticleColor{ 1 };
  glm::vec4 maxParticleColor{ 1 };
  float interval{ 0.1f };
  float minLife{ 1 };
  float maxLife{ 1 };
  int maxParticles{ 100 };
  bool fly = false;
};