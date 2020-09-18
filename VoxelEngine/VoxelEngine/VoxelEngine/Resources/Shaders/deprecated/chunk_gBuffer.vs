#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aShininess;

out vec4 vColor;
out vec3 vNormal;
out vec3 vPos;
out float vShininess;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  vShininess = aShininess;
  vPos = vec3(model * vec4(aPos, 1.0));
  vColor = aColor;
  vNormal = transpose(inverse(mat3(model))) * aNormal;
  
  //gl_Position = projection * view * model * vec4(aPos, 1.0);
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}