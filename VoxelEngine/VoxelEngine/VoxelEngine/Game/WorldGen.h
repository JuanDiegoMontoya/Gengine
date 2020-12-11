#pragma once
#include <queue>
#include <glm/glm.hpp>

class VoxelManager;

class WorldGen2
{
public:
  WorldGen2(VoxelManager& v) : vm(v) {}
  void Init();
  void GenerateWorld();
  void InitMeshes();
  void InitBuffers();
  void InitializeSunlight();
private:
  VoxelManager& vm;
  std::queue<glm::ivec3> lightsToPropagate;

  void sunlightPropagateOnce(const glm::ivec3& wpos);
  bool checkDirectSunlight(glm::ivec3 wpos);
};