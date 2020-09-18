#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aShininess;
//layout (location = 4) in float aSunlight;
layout (location = 4) in int aLighting;

const int NUM_CASCADES = 3;

uniform mat4 lightSpaceMatrix[NUM_CASCADES];
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;
out float vShininess;
//out float vSunlight;
out lowp vec4 vLighting;
out vec4 FragPosLightSpace[NUM_CASCADES];
out float ClipSpacePosZ;

void main()
{
  vShininess = aShininess;
  //vSunlight = aSunlight;
  vLighting.r = aLighting >> 12;
  vLighting.g = (aLighting >> 8) & 0xF;
  vLighting.b = (aLighting >> 4) & 0xF;
  vLighting.a = aLighting & 0xF;
  vLighting = 1 + (vLighting / 16.0);
  vPos = vec3(u_model * vec4(aScreenPos, 1.0));
  vColor = aColor;
  vNormal = transpose(inverse(mat3(u_model))) * aNormal;
  for (int i = 0; i < NUM_CASCADES; i++)
    FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(vPos, 1.0);
    //FragPosLightSpace[i] = lightSpaceMatrix[i] * u_model * vec4(aScreenPos, 1.0);
  
  gl_Position = u_proj * u_view * u_model * vec4(aScreenPos, 1.0);
  ClipSpacePosZ = gl_Position.z;
}