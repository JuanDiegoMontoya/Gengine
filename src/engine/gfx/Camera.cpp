#include "../PCH.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

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
    return glm::lookAt(position, position + GetForwardDir(), Constants::Up);
  }
}