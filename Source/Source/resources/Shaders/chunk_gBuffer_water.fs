#version 450 core

#define VISUALIZE_MAPS 0

// texture buffers
#if VISUALIZE_MAPS
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
#else
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
#endif
layout (location = 2) out vec4 gAlbedoSpec;

in vec4 vColor;
in vec3 vNormal;
in vec3 vPos;
in float vShininess;


void main()
{
#if VISUALIZE_MAPS
  gNormal = vec4(normalize(vNormal) * .5 + .5, 1.0);
  gPosition = vec4(log2(vPos) / 10.0, 1.0);
#else
  gPosition = vPos;
  gNormal = normalize(vNormal);
#endif
  gAlbedoSpec.rgb = vColor.rgb;
  gAlbedoSpec.a = vShininess;
}