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
uniform float u_time;

// ssr
//out vec2 ssTexCoords;
uniform sampler2D ssr_positions;
out vec3 camNormal;

out vec3 vPos;
out vec4 vColor;
out vec3 vNormal;
out float vShininess;
//out float vSunlight;
out vec4 vLighting;
out vec4 FragPosLightSpace[NUM_CASCADES];
out float ClipSpacePosZ;

vec2 computeSSTexCoord()
{
  vec4 ndc = gl_Position;
  vec2 scrSize = textureSize(ssr_positions, 0);
  ndc /= ndc.w; // perspective divide
  // the "+0" is where the viewport start point would be
  ndc.x = (ndc.x + 1.0) * scrSize.x / 2.0 + 0; // viewport transformation
  ndc.y = (ndc.y + 1.0) * scrSize.y / 2.0 + 0; // viewport transformation
  return ndc.xy;
}

float fade(float t) { return t * t * t * (t * (t * 6. - 15.) + 10.); }
vec2 smoothy(vec2 x) { return vec2(fade(x.x), fade(x.y)); }

vec2 hash(vec2 co)
{
  float m = dot(co, vec2(12.9898, 78.233));
  return fract(vec2(sin(m),cos(m))* 43758.5453) * 2. - 1.;
}

float perlinNoise(vec2 uv)
{
  vec2 PT  = floor(uv);
  vec2 pt  = fract(uv);
  vec2 mmpt= smoothy(pt);

  vec4 grads = vec4(
    dot(hash(PT + vec2(.0, 1.)), pt-vec2(.0, 1.)), dot(hash(PT + vec2(1., 1.)), pt-vec2(1., 1.)),
    dot(hash(PT + vec2(.0, .0)), pt-vec2(.0, .0)), dot(hash(PT + vec2(1., .0)), pt-vec2(1., 0.))
  );

  return 5.*mix (mix (grads.z, grads.w, mmpt.x), mix (grads.x, grads.y, mmpt.x), mmpt.y);
}

// makes the noise look jittery; currently unused
float fbm(vec2 uv)
{
  float finalNoise = 0.;
  finalNoise += .50000*perlinNoise(2.*uv);
  finalNoise += .25000*perlinNoise(4.*uv);
  finalNoise += .12500*perlinNoise(8.*uv);
  finalNoise += .06250*perlinNoise(16.*uv);
  finalNoise += .03125*perlinNoise(32.*uv);

  return finalNoise;
}

float ripplePos(float x, float z)
{
  //return sin(u_time * 2) * (sin(x) + cos(z)) * .5;
  return (perlinNoise(vec2(x / 10. + u_time / 3, z / 10. + u_time / 3)) * .4 + // primary wave
          perlinNoise(vec2(-x / 10. + u_time / 3, -z / 10. + u_time / 3)) * .2) / 1.0 + // secondary wave
          perlinNoise(vec2(x / 25. + u_time / 2, z / 25. + u_time / 2)) * .2; // detail wave
  //return perlinNoise(vec2(x / 10., z / 10.)) * 0;// + sin(u_time * 2);
  //return hash(vec2(x, z + u_time)).x;
  //return 0;
  //return fbm(vec2(x / 10. + u_time / 5, z / 10. + u_time / 5)) * 0.4;// + sin(u_time * 2);
}

// sample the heightmap in 3 places to obtain the normal
vec3 rippleNormal(vec2 pos0)
{
  vec2 pos1 = pos0 + vec2(1, 0);
  vec2 pos2 = pos0 + vec2(0, 1);
  
  float yp0 = ripplePos(pos0.x, pos0.y);
  float yp1 = ripplePos(pos1.x, pos1.y);
  float yp2 = ripplePos(pos2.x, pos2.y);
  
  vec3 rp0 = vec3(pos0.x, yp0, pos0.y);
  vec3 rp1 = vec3(pos1.x, yp1, pos1.y);
  vec3 rp2 = vec3(pos2.x, yp2, pos2.y);
  
  vec3 ln1 = rp1 - rp0;
  vec3 ln2 = rp2 - rp0;
  
  return cross(ln2, ln1);
}

void main()
{
  // TODO: lighting stuff
  vShininess = aShininess;
  //vSunlight = aSunlight;
  vLighting.r = aLighting >> 12;
  vLighting.g = (aLighting >> 8) & 0xF;
  vLighting.b = (aLighting >> 4) & 0xF;
  vLighting.a = aLighting & 0xF;
  vLighting = 1 + (vLighting / 16.0);
  vPos = vec3(u_model * vec4(aScreenPos, 1.0));
  float rip = ripplePos(vPos.x, vPos.z) * .5 + .5;
  
  // translation matrix
  mat4 ripTran = mat4(
  1.0, 0.0, 0.0, 0.0, 
  0.0, 1.0, 0.0, 0.0, 
  0.0, 0.0, 1.0, 0.0, 
  0, -(rip), 0.0, 1.0);
  
  vColor = aColor;
  vNormal = transpose(inverse(mat3(u_model))) * aNormal;
  vNormal += rippleNormal(vPos.xz);
  camNormal = normalize(aNormal + rippleNormal(vPos.xz));//rippleNormal(vPos.xz);
  for (int i = 0; i < NUM_CASCADES; i++)
    FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(vPos, 1.0);
  
  gl_Position = u_proj * u_view * u_model * ripTran * vec4(aScreenPos, 1.0);
  ClipSpacePosZ = gl_Position.z;
  //ssTexCoords = computeSSTexCoord();
}