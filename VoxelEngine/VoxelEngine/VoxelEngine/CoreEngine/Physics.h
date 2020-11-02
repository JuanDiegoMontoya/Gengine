#pragma once
#include <glm/glm.hpp>

namespace physx
{
  class PxRigidActor;
}

namespace Physics
{
  struct BoxCollider
  {
    BoxCollider(const glm::vec3& halfextents) : halfExtents(halfextents) {}
    glm::vec3 halfExtents;
  };

  class PhysicsManager
  {
  public:
    physx::PxRigidActor* RegisterObject();

  private:

  };
}