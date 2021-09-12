#version 460 core
layout (location = 0) in vec3 vTexCoord;

layout (location = 2) uniform samplerCube u_skybox;

layout (location = 0) out vec4 fragColor;

void main()
{
	gl_FragDepth = 0.0;
	fragColor = vec4(texture(u_skybox, vTexCoord).rgb, 1.0);
}