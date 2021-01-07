#version 460 core

layout (location = 0) uniform bool u_gammaCorrection = false;
layout (location = 1) uniform sampler2D u_hdrBuffer;
layout (location = 2) uniform float u_exposureFactor = 1.0;

layout (location = 0) in vec2 vTexCoord;
layout (location = 0) out vec4 fragColor;

layout (std430, binding = 0) readonly buffer exposures
{
  float readExposure;
  float writeExposure;
};

void main()
{
  const float gamma = 2.2;
  
  // Reinhard tonemapping operator 
  vec3 hdrColor = texture(u_hdrBuffer, vTexCoord).rgb;
  vec3 mapped = vec3(1.0) - exp(-hdrColor * u_exposureFactor * readExposure);
  
  if (u_gammaCorrection == true)
  {
    mapped = pow(mapped, vec3(1.0 / gamma));
  }

  //fragColor = vec4(mapped * .001 + vec3(vTexCoord, 0.0), 1.0);
  fragColor = vec4(mapped, 1.0);
}