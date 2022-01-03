#include "../PCH.h"
#include "Camera.h"
#include <utility/MathExtensions.h>

namespace GFX
{
  glm::vec3 View::GetForwardDir() const
  {
    return glm::vec3
    {
      cos(pitch) * cos(yaw),
      sin(pitch),
      cos(pitch) * sin(yaw)
    };
  }

  glm::mat4 View::GetViewMatrix() const
  {
    return glm::lookAt(position, position + GetForwardDir(), Constants::Cardinals[static_cast<uint32_t>(upDir)]);
  }

  void View::SetForwardDir(glm::vec3 dir)
  {
    ASSERT(glm::abs(1.0f - glm::length(dir)) < 0.0001f);
    pitch = glm::asin(dir.y);
    yaw = glm::acos(dir.x / glm::cos(pitch));
    if (dir.x >= 0 && dir.z < 0)
      yaw *= -1;
  }

  glm::mat4 ProjectionInfo::GetProjMatrix() const
  {
    return MakeInfReversedZProjRH(info.fovyRadians, info.aspectRatio, info.nearPlane);
  }
}