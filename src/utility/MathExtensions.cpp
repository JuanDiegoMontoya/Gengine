#include "MathExtensions.h"
#include <cmath>

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear)
{
  float f = 1.0f / tan(fovY_radians / 2.0f);
  return glm::mat4(
    f / aspectWbyH, 0.0f, 0.0f, 0.0f,
    0.0f, f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, zNear, 0.0f);
}