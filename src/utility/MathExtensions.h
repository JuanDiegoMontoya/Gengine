#pragma once
#include <glm/mat4x4.hpp>
#include <glm/fwd.hpp>

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear);