#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vPos;
out vec2 vTexCoords;

void main()
{
    vPos = vec3(model * vec4(aPos, 1.0));
    vNormal = mat3(transpose(inverse(model))) * aNormal;  
    vTexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(vPos, 1.0);
}