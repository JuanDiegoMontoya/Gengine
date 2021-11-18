#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) uniform mat4 u_viewProj;

layout(location = 0) out VS_OUT
{
  vec3 posWorldSpace;
  vec3 normal;
  vec2 texCoord;
}vs_out;

struct UniformData
{
  mat4 model;
};

layout(std430, binding = 0) readonly buffer data
{
  UniformData uniforms[];
};

void main()
{
  mat4 model = uniforms[gl_BaseInstance + gl_InstanceID].model;
  vs_out.posWorldSpace = vec3(model * vec4(aPos, 1.0f));
  //vs_out.normal = (model * vec4(aNormal, 0.0)).xyz;
	vs_out.normal = aNormal;
  vs_out.texCoord = aTexCoord;

  gl_Position = u_viewProj * vec4(vs_out.posWorldSpace, 1.0f);
}