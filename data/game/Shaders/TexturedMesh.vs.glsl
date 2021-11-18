#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) uniform mat4 u_mvp;
layout(location = 1) uniform mat4 u_model;
layout(location = 2) uniform mat4 u_normalMatrix;

layout(location = 0) out VS_OUT
{
  vec3 posWorldSpace;
  vec3 normal;
  vec2 texCoord;
}vs_out;

void main()
{
  vs_out.posWorldSpace = vec3(u_model * vec4(aPos, 1.0f));
  vs_out.normal = normalize(u_normalMatrix * vec4(aNormal, 0.0)).xyz; 
  vs_out.texCoord = aTexCoord;

  gl_Position = u_mvp * vec4(aPos, 1.0f);
}