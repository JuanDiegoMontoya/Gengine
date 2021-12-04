#version 460 core

layout(location = 0) in VS_OUT
{
  vec3 posWorldSpace;
  vec3 normal;
  vec2 texCoord;
}fs_in;

layout(binding = 0) uniform sampler2D u_tex;
layout(location = 4) uniform float u_time = 0;

layout(location = 0) out vec4 o_diffuse;
layout(location = 1) out vec4 o_normal;
layout(location = 2) out vec4 o_pbr;

void main()
{
  vec3 texColor = texture(u_tex, fs_in.texCoord).rgb;
  vec3 normalColor = fs_in.normal * .5 + .5;
  o_diffuse = vec4(mix(texColor, normalColor, fs_in.posWorldSpace / 100), 1.0);
  o_diffuse.rgb *= abs(sin(u_time));
  //o_normal = vec4(normalize(fs_in.normal), 1.0);
  o_normal = vec4(normalize(cross(dFdx(fs_in.posWorldSpace), dFdy(fs_in.posWorldSpace))), 1.0);
  o_pbr = vec4(0.0, 0.0, 0.0, 1.0);
}