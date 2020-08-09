#version 450 core

// texture buffers
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec4 vColor;
in vec3 vNormal;
in vec3 vPos;
in float vShininess;

void main()
{
  //gPosition = vec4(normalize(vPos) * .5 + .5, 1.0);
  //gNormal = vec4(normalize(vNormal) * .5 + .5, 1.0);
  gPosition = vec4(vPos, 1.0);
  gNormal = vec4(normalize(vNormal), 1.0);
  gAlbedoSpec.rgb = vColor.rgb;
  gAlbedoSpec.a = vShininess;
}