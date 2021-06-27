#include "PCH.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/Frustum.h>


Frustum::~Frustum() 
{
    // cokc
}

void Frustum::Transform(const glm::mat4& proj, const glm::mat4& view)
{
  glm::mat4 clip;
  clip[0][0] = view[0][0] * proj[0][0] + view[0][1] * proj[1][0] + view[0][2] * proj[2][0] + view[0][3] * proj[3][0];
  clip[0][1] = view[0][0] * proj[0][1] + view[0][1] * proj[1][1] + view[0][2] * proj[2][1] + view[0][3] * proj[3][1];
  clip[0][2] = view[0][0] * proj[0][2] + view[0][1] * proj[1][2] + view[0][2] * proj[2][2] + view[0][3] * proj[3][2];
  clip[0][3] = view[0][0] * proj[0][3] + view[0][1] * proj[1][3] + view[0][2] * proj[2][3] + view[0][3] * proj[3][3];

  clip[1][0] = view[1][0] * proj[0][0] + view[1][1] * proj[1][0] + view[1][2] * proj[2][0] + view[1][3] * proj[3][0];
  clip[1][1] = view[1][0] * proj[0][1] + view[1][1] * proj[1][1] + view[1][2] * proj[2][1] + view[1][3] * proj[3][1];
  clip[1][2] = view[1][0] * proj[0][2] + view[1][1] * proj[1][2] + view[1][2] * proj[2][2] + view[1][3] * proj[3][2];
  clip[1][3] = view[1][0] * proj[0][3] + view[1][1] * proj[1][3] + view[1][2] * proj[2][3] + view[1][3] * proj[3][3];

  clip[2][0] = view[2][0] * proj[0][0] + view[2][1] * proj[1][0] + view[2][2] * proj[2][0] + view[2][3] * proj[3][0];
  clip[2][1] = view[2][0] * proj[0][1] + view[2][1] * proj[1][1] + view[2][2] * proj[2][1] + view[2][3] * proj[3][1];
  clip[2][2] = view[2][0] * proj[0][2] + view[2][1] * proj[1][2] + view[2][2] * proj[2][2] + view[2][3] * proj[3][2];
  clip[2][3] = view[2][0] * proj[0][3] + view[2][1] * proj[1][3] + view[2][2] * proj[2][3] + view[2][3] * proj[3][3];

  clip[3][0] = view[3][0] * proj[0][0] + view[3][1] * proj[1][0] + view[3][2] * proj[2][0] + view[3][3] * proj[3][0];
  clip[3][1] = view[3][0] * proj[0][1] + view[3][1] * proj[1][1] + view[3][2] * proj[2][1] + view[3][3] * proj[3][1];
  clip[3][2] = view[3][0] * proj[0][2] + view[3][1] * proj[1][2] + view[3][2] * proj[2][2] + view[3][3] * proj[3][2];
  clip[3][3] = view[3][0] * proj[0][3] + view[3][1] * proj[1][3] + view[3][2] * proj[2][3] + view[3][3] * proj[3][3];

  data_[Right][A] = clip[0][3] - clip[0][0];
  data_[Right][B] = clip[1][3] - clip[1][0];
  data_[Right][C] = clip[2][3] - clip[2][0];
  data_[Right][D] = clip[3][3] - clip[3][0];
  Normalize(Plane::Right);

  data_[Left][A] = clip[0][3] + clip[0][0];
  data_[Left][B] = clip[1][3] + clip[1][0];
  data_[Left][C] = clip[2][3] + clip[2][0];
  data_[Left][D] = clip[3][3] + clip[3][0];
  Normalize(Plane::Left);

  data_[Bottom][A] = clip[0][3] + clip[0][1];
  data_[Bottom][B] = clip[1][3] + clip[1][1];
  data_[Bottom][C] = clip[2][3] + clip[2][1];
  data_[Bottom][D] = clip[3][3] + clip[3][1];
  Normalize(Plane::Bottom);

  data_[Top][A] = clip[0][3] - clip[0][1];
  data_[Top][B] = clip[1][3] - clip[1][1];
  data_[Top][C] = clip[2][3] - clip[2][1];
  data_[Top][D] = clip[3][3] - clip[3][1];
  Normalize(Plane::Top);

  data_[Front][A] = clip[0][3] - clip[0][2];
  data_[Front][B] = clip[1][3] - clip[1][2];
  data_[Front][C] = clip[2][3] - clip[2][2];
  data_[Front][D] = clip[3][3] - clip[3][2];
  Normalize(Plane::Front);

  data_[Back][A] = clip[0][3] + clip[0][2];
  data_[Back][B] = clip[1][3] + clip[1][2];
  data_[Back][C] = clip[2][3] + clip[2][2];
  data_[Back][D] = clip[3][3] + clip[3][2];
  Normalize(Plane::Back);
}

void Frustum::Normalize(Plane plane)
{
  float magnitude = glm::sqrt(
    data_[plane][A] * data_[plane][A] +
    data_[plane][B] * data_[plane][B] +
    data_[plane][C] * data_[plane][C]
  );

  data_[plane][A] /= magnitude;
  data_[plane][B] /= magnitude;
  data_[plane][C] /= magnitude;
  data_[plane][D] /= magnitude;
}

Frustum::Visibility Frustum::IsInside(const glm::vec3& point) const
{
  for (unsigned int i = 0; i < 6; i++)
  {
    if (data_[i][A] * point.x +
      data_[i][B] * point.y +
      data_[i][C] * point.z +
      data_[i][D] <= 0)
    {
      return Visibility::Invisible;
    }
  }

  return Visibility::Full;
}

Frustum::Visibility Frustum::IsInside(AABB box) const
{
  auto GetVisibility = [](const glm::vec4& clip, AABB box)
  {
    float x0 = box.min.x * clip.x;
    float x1 = box.max.x * clip.x;
    float y0 = box.min.y * clip.y;
    float y1 = box.max.y * clip.y;
    float z0 = box.min.z * clip.z + clip.w;
    float z1 = box.max.z * clip.z + clip.w;
    float p1 = x0 + y0 + z0;
    float p2 = x1 + y0 + z0;
    float p3 = x1 + y1 + z0;
    float p4 = x0 + y1 + z0;
    float p5 = x0 + y0 + z1;
    float p6 = x1 + y0 + z1;
    float p7 = x1 + y1 + z1;
    float p8 = x0 + y1 + z1;

    if (p1 <= 0 && p2 <= 0 && p3 <= 0 && p4 <= 0 && p5 <= 0 && p6 <= 0 && p7 <= 0 && p8 <= 0)
    {
      return Visibility::Invisible;
    }
    if (p1 > 0 && p2 > 0 && p3 > 0 && p4 > 0 && p5 > 0 && p6 > 0 && p7 > 0 && p8 > 0)
    {
      return Visibility::Full;
    }

    return Visibility::Partial;
  };

  Visibility v0 = GetVisibility(GetPlane(Right), box);
  if (v0 == Visibility::Invisible)
  {
    return Visibility::Invisible;
  }

  Visibility v1 = GetVisibility(GetPlane(Left), box);
  if (v1 == Visibility::Invisible)
  {
    return Visibility::Invisible;
  }

  Visibility v2 = GetVisibility(GetPlane(Bottom), box);
  if (v2 == Visibility::Invisible)
  {
    return Visibility::Invisible;
  }

  Visibility v3 = GetVisibility(GetPlane(Top), box);
  if (v3 == Visibility::Invisible)
  {
    return Visibility::Invisible;
  }

  Visibility v4 = GetVisibility(GetPlane(Front), box);
  if (v4 == Visibility::Invisible)
  {
    return Visibility::Invisible;
  }

  if (v0 == Visibility::Full && v1 == Visibility::Full &&
    v2 == Visibility::Full && v3 == Visibility::Full &&
    v4 == Visibility::Full)
  {
    return Visibility::Full;
  }

  return Visibility::Partial;
}