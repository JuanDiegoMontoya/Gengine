#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) uniform mat4 u_viewProj;

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNormal;
layout (location = 2) out vec2 vTexCoord;

struct UniformData
{
	mat4 model;
};
layout (std430, binding = 0) readonly buffer data
{
	UniformData uniforms[];
};

void main()
{
	mat4 model = uniforms[gl_BaseInstance + gl_InstanceID].model;
	vTexCoord = aTexCoord;

	vNormal = aNormal;
	vPos = vec3(model * vec4(aPos, 1.0f));

	gl_Position = u_viewProj * vec4(vPos, 1.0f);
}