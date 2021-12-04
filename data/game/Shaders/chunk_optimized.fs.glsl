#version 460 core

layout(location = 1) uniform vec3 u_viewPos;
layout(location = 2) uniform vec3 u_envColor = vec3(1.0);
layout(location = 3) uniform float u_minBrightness = 0.01;
layout(location = 4) uniform float u_ambientOcclusionStrength = 0.5;
layout(binding = 0) uniform sampler2DArray u_diffuseTextures;
layout(binding = 1) uniform sampler2DArray u_normalTextures;
layout(binding = 2) uniform sampler2DArray u_PBRTextures;

// material properties
layout(location = 0) in VS_OUT
{
  vec3 posViewSpace;
  vec3 texCoord;
  vec4 lighting;
  flat uint quadAO;
  vec3 normal;
}fs_in;

layout(location = 0) out vec4 o_diffuse;
layout(location = 1) out vec3 o_normal;
layout(location = 2) out vec2 o_pbr;

// dithered transparency
bool clipTransparency(float alpha)
{
  int x = (int(gl_FragCoord.x)) % 4;
  int y = (int(gl_FragCoord.y)) % 4;

  const mat4 thresholdMatrix = mat4
  (
    1.0 / 16.0,  9.0 / 16.0,  3.0 / 16.0, 11.0 / 16.0,
    13.0 / 16.0,  5.0 / 16.0, 15.0 / 16.0,  7.0 / 16.0,
    4.0 / 16.0, 12.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
    16.0 / 16.0,  8.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0
  );

  float limit = thresholdMatrix[x][y];

  // Is this pixel below the opacity limit? Skip drawing it
  return alpha < limit;
}

vec3 GetNormal()
{
  return normalize(cross(dFdx(fs_in.posViewSpace), dFdy(fs_in.posViewSpace)));
  //return normalize(fs_in.normal);
}

float CalculateAO()
{
  // bilinearly interpolate AO across face
  uint ao1i = (fs_in.quadAO >> 0) & 0x3;
  uint ao2i = (fs_in.quadAO >> 2) & 0x3;
  uint ao3i = (fs_in.quadAO >> 4) & 0x3;
  uint ao4i = (fs_in.quadAO >> 6) & 0x3;
  float ao1f = float(ao1i) / 3.0;
  float ao2f = float(ao2i) / 3.0;
  float ao3f = float(ao3i) / 3.0;
  float ao4f = float(ao4i) / 3.0;

  float r = mix(ao1f, ao2f, fs_in.texCoord.y);
  float l = mix(ao4f, ao3f, fs_in.texCoord.y);
  float v = mix(l, r, fs_in.texCoord.x);
  //float edgeFactor = 
  return v;
}

void main()
{
  vec4 texColor = texture(u_diffuseTextures, fs_in.texCoord).rgba;
  vec3 normal = GetNormal();

  // dithering
  if ((texColor.a < 1.0 && clipTransparency(texColor.a) || texColor.a == 0.0))
  {
    discard;
  }

  vec3 diffuse = texColor.rgb;
  vec3 envLight = fs_in.lighting.a * u_envColor;
  vec3 shaded = diffuse * max(fs_in.lighting.rgb, envLight);
  shaded = max(shaded, vec3(u_minBrightness));
  shaded = mix(shaded, shaded * CalculateAO(), u_ambientOcclusionStrength);

  bool isShiny = abs(fs_in.texCoord.z - 3.0) <= 0.001 || abs(fs_in.texCoord.z - 7.0) <= 0.001;

  vec2 pbr = texture(u_PBRTextures, fs_in.texCoord).xy;
  o_pbr = pbr;
  o_normal = GetNormal();
  o_diffuse = vec4(shaded, 1.0);
}