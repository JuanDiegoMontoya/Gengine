#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_mvp;

void main()
{
    //gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    v_texCoord = texCoord;
    gl_Position = u_mvp * vec4(aPos.xyz, 1.0f);
}
