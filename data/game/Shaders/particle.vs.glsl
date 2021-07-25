#version 460 core

#include "particle.h"

layout(std430, binding = 0) readonly buffer ParticlesShareds
{
  ParticleSharedData particlesShared[];
};

layout(std430, binding = 1) readonly buffer ParticlesUpdate
{
  ParticleUpdateData particlesUpdate[];
};

layout(std430, binding = 2) readonly buffer ParticlesRender
{
  ParticleRenderData particlesRender[];
};

layout(std430, binding = 3) readonly buffer Drawindices
{
  uint drawIndices[];
};

layout(location = 0) uniform mat4 u_viewProj;
layout(location = 1) uniform vec3 u_cameraRight;
layout(location = 2) uniform vec3 u_cameraUp;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;

// vertices in [0, 1]
vec2 CreateQuad(in uint vertexID) // triangle fan
{
  uint b = 1 << vertexID;
  return vec2((0x3 & b) != 0, (0x9 & b) != 0);
}

void main()
{
  vTexCoord = CreateQuad(gl_VertexID);
  vec2 aPos = vTexCoord - 0.5;

  int index = int(drawIndices[gl_InstanceID]);
  //int index = gl_InstanceID;

  ParticleSharedData psd = particlesShared[index];
  //if (psd.position_A.w != 1.0)
  //{
  //  gl_Position = vec4(0.0);
  //  return;
  //}
  ParticleRenderData prd = particlesRender[index];

  vec2 scale = unpackHalf2x16(prd.packedScaleX_packedColorY.x);
  vColor = unpackUnorm4x8(prd.packedScaleX_packedColorY.y);

  vec3 vertexPosition_worldspace =
    psd.position_A.xyz +
    u_cameraRight * aPos.x * scale.x +
    u_cameraUp * aPos.y * scale.y;

  gl_Position = u_viewProj * vec4(vertexPosition_worldspace, 1.0);
  //gl_Position = u_viewProj * u_model * vec4((vec3(aPos, 0.0) * vec3(particle.scale.xy, 0.0)) + particle.pos.xyz, 1.0);
}