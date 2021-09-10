#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace GFX
{
  namespace Constants
  {
    constexpr glm::vec3 Up{ 0, 1, 0 };
  }

  struct View
  {
    glm::vec3 position{};
    float pitch{}; // pitch angle in radians
    float yaw{};   // yaw angle in radians
    //float roll{};  // roll angle in radians

    glm::vec3 GetForwardDir() const;
    glm::mat4 GetViewMatrix() const;
  };

  struct Camera
  {
    View viewInfo{};
    glm::mat4 proj{};

    glm::mat4 GetViewProj() const { return proj * viewInfo.GetViewMatrix(); }
  };
}