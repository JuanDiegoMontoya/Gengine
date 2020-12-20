#version 450 core

// material properties
//in vec4 vColor;
in vec3 vPos;
in vec3 vNormal;
in vec3 vTexCoord;
in vec4 vLighting; // RGBSun
in vec3 vBlockPos;

layout(location = 1) uniform vec3 viewPos;   // world space
layout(location = 2) uniform float sunAngle; // cos sun angle to normal of horizon, 0-1
layout(location = 3) uniform sampler2D textureAtlas;

layout(location = 4) uniform float fogStart; // world space
layout(location = 5) uniform float fogEnd;   // world space
layout(location = 6) uniform vec3 fogColor;

layout(location = 7) uniform sampler2DArray textures;
layout(location = 8) uniform sampler2D blueNoise;

out vec4 fragColor;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

// returns intensity of fog, from 0 to 1
float FogCalculation()
{
  float dist = distance(vPos, viewPos);
  return clamp(map(dist, fogStart, fogEnd, 0.0, 1.0), 0.0, 1.0);
}

// noise from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float mod289(float x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 perm(vec4 x) { return mod289(((x * 34.0) + 1.0) * x); }
float noise(vec3 p)
{
  vec3 a = floor(p);
  vec3 d = p - a;
  d = d * d * (3.0 - 2.0 * d);

  vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
  vec4 k1 = perm(b.xyxy);
  vec4 k2 = perm(k1.xyxy + b.zzww);

  vec4 c = k2 + a.zzzz;
  vec4 k3 = perm(c);
  vec4 k4 = perm(c + 1.0);

  vec4 o1 = fract(k3 * (1.0 / 41.0));
  vec4 o2 = fract(k4 * (1.0 / 41.0));

  vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
  vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

  return o4.y * d.y + o4.x * (1.0 - d.y);
}

vec3 random3(vec3 c)
{
  float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
  vec3 r;
  r.z = fract(512.0*j);
  j *= .125;
  r.x = fract(512.0*j);
  j *= .125;
  r.y = fract(512.0*j);
  return r-0.5;
}


#define RNG_OFFSET 1
#define USE_KERNEL 0 // if false, will use blue noise
// dithered transparency
bool clipTransparency(float alpha)
{
#if RNG_OFFSET  // rng offset
  ivec2 offset = ivec2(500 * random3(vBlockPos).xy);
#else
  ivec2 offset = ivec2(0, 0);
#endif // rng offset

#if USE_KERNEL // kernel dithering if true
  int x = (int(gl_FragCoord.x) + offset.x) % 4;
  int y = (int(gl_FragCoord.y) + offset.y) % 4;
  int index = x + y * 4;
  float limit = 0.0;

  const mat4 thresholdMatrix = mat4
  (
    1.0 / 16.0,  9.0 / 16.0,  3.0 / 16.0, 11.0 / 16.0,
    13.0 / 16.0,  5.0 / 16.0, 15.0 / 16.0,  7.0 / 16.0,
    4.0 / 16.0, 12.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
    16.0 / 16.0,  8.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0
  );
  limit = thresholdMatrix[x][y] + texture(blueNoise, vec2(x, y)).x * .001;
#else
  ivec2 md = textureSize(blueNoise, 0);
  vec2 coord = gl_FragCoord.xy + offset;
  ivec2 uv = ivec2(int(coord.x) % md.x, int(coord.y) % md.y);
  float limit = texture(blueNoise, vec2(uv) / vec2(md)).r;
#endif // kernel dithering

  // Is this pixel below the opacity limit? Skip drawing it
  return alpha < limit;
}


void main()
{
  vec4 texColor = texture(textures, vTexCoord).rgba;

  // dithering happens here
  if (texColor.a < 1 && (clipTransparency(texColor.a) || texColor.a == 0))
    discard;

  vec3 tempColor = texColor.rgb;

  vec4 lighting = vLighting;
  lighting.a *= sunAngle;
  lighting = max(lighting, vec4(.01));
  tempColor *= max(lighting.rgb, lighting.aaa);
  // fog is applied last
  tempColor = mix(tempColor, fogColor, FogCalculation());
  fragColor = vec4(tempColor, 1.0); // alpha is always 100% or 0% (per fragment)
  //fragColor.xyz = fragColor.xyz * .1 + random3(vBlockPos / 10).xyz;
  //fragColor.rgb = texColor + fragColor.rgb * .0001;
  //fragColor = vec4(.0001 * tempColor + lighting.aaa, 1.0);
}