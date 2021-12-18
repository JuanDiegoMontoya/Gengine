#ifndef COMMON_H
#define COMMON_H

#define DEPTH_ZERO_TO_ONE

// Returns ±1
vec2 signNotZero(vec2 v)
{
  return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

// Assume normalized input. Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(in vec3 v)
{
  // Project the sphere onto the octahedron, and then onto the xy plane
  vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
  // Reflect the folds of the lower hemisphere over the diagonals
  return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

vec3 oct_to_float32x3(vec2 e)
{
  vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
  if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
  return normalize(v);
}

vec3 UnprojectUV(float depth, vec2 uv, mat4 invXProj)
{
#ifndef DEPTH_ZERO_TO_ONE
  float z = depth * 2.0 - 1.0; // [0, 1] -> [-1, 1]
#else
  float z = depth;
#endif
  vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0); // [0, 1] -> [-1, 1]

  // undo view + projection
  vec4 worldSpacePosition = invXProj * clipSpacePosition;
  worldSpacePosition /= worldSpacePosition.w;

  return worldSpacePosition.xyz;
}

#endif