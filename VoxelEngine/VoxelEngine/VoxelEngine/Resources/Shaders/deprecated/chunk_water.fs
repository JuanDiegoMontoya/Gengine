#version 450 core

struct DirLight
{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
const int NUM_CASCADES = 3;

// material properties
in vec4 vColor; // since there will be no textures, this is the diffuse component
in vec3 vNormal;
in float vShininess;
//in float vSunlight;
in vec4 vLighting;
in vec3 vPos;
in vec4 FragPosLightSpace[NUM_CASCADES];
in float ClipSpacePosZ;

uniform sampler2D shadowMap[NUM_CASCADES];
uniform float cascadeEndClipSpace[NUM_CASCADES];
uniform DirLight dirLight; // the sun
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float u_time;

uniform float fogStart; // world
uniform float fogEnd;   // world
uniform vec3 fogColor;

// ssr
//in vec2 ssTexCoords;
in vec3 camNormal; // view space normal
vec2 ssTexCoords;
uniform sampler2D ssr_positions;
uniform sampler2D ssr_normals;
uniform sampler2D ssr_albedoSpec;
uniform sampler2D ssr_depth;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec3 ssr_skyColor;
uniform mat4 inv_projection;
uniform bool computeSSR;
uniform bool computeShadow;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);


float near_plane = .1;
float far_plane = 500.;
float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

// calculate view position of current fragment
vec3 calcViewPosition(in vec2 texCoord)
{
  // Combine UV & depth into XY & Z (NDC)
  vec3 rawPosition = vec3(texCoord, gl_FragCoord.z);
  
  // Convert from (0, 1) range to (-1, 1)
  vec4 ScreenSpacePosition = vec4(rawPosition * 2. - 1., 1.);
  
  // Undo Perspective transformation to bring into view space
  vec4 ViewPosition = inv_projection * ScreenSpacePosition;
  
  // Perform perspective divide and return
  return ViewPosition.xyz / ViewPosition.w;
}

// calculate view position with depth buffer
vec3 calcViewPositionDepthTex(in vec2 texCoord)
{
  vec3 rawPosition = vec3(texCoord, texture(ssr_depth, texCoord).r);
  vec4 ScreenSpacePosition = vec4(rawPosition * 2. - 1., 1.);
  vec4 ViewPosition = inv_projection * ScreenSpacePosition;
  return ViewPosition.xyz / ViewPosition.w;
}

vec2 binarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth)
{
  float depth;

  vec4 projectedCoord;

  for(int i = 0; i < 30; i++)
  {
    projectedCoord = u_proj * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

    depth = calcViewPositionDepthTex(projectedCoord.xy).z;

    dDepth = hitCoord.z - depth;

    dir *= 0.5;
    if(dDepth > 0.0)
      hitCoord += dir;
    else
      hitCoord -= dir;    
  }

  projectedCoord = u_proj * vec4(hitCoord, 1.0);
  projectedCoord.xy /= projectedCoord.w;
  projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

  return vec2(projectedCoord.xy);
}

vec2 rayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
  dir *= .3;

  for (int i = 0; i < 40; i++)
  {
    hitCoord += dir; 
    
    vec4 projectedCoord = u_proj * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5; 

    float depth = calcViewPositionDepthTex(projectedCoord.xy).z;
    dDepth = hitCoord.z - depth;

    if(dDepth < 0.0)
      return binarySearch(dir, hitCoord, dDepth);
  }

  return vec2(-1.0f);
}

vec3 ssr()
{
  // compute texture coordinates
  ssTexCoords.x = gl_FragCoord.x / textureSize(ssr_positions, 0).x;
  ssTexCoords.y = gl_FragCoord.y / textureSize(ssr_positions, 0).y;
  vec2 texCoord = ssTexCoords;
  
  // normal at initial ray position
  vec3 normal = camNormal;
  vec3 fViewPos = calcViewPosition(texCoord);
  
  // Reflection vector
  vec3 reflected = normalize(reflect(normalize(fViewPos), normalize(normal)));
  
  // Ray cast
  vec3 hitPos = fViewPos;
  float dDepth;
  float minRayStep = .05f;
  vec2 coords = rayCast(reflected * max(minRayStep, -fViewPos.z), hitPos, dDepth);
  if (coords != vec2(-1.0))
    return mix(vColor.rgb, texture(ssr_albedoSpec, coords).rgb / 3, .7005);
  else
    return vColor.rgb; // ray failed to intersect (use sky color in the future)
}










// WATER NOISE STUFF
float fade(float t) { return t * t * t * (t * (t * 6. - 15.) + 10.); }
vec2 smoothy(vec2 x) { return vec2(fade(x.x), fade(x.y)); }

vec2 hash2(vec2 co)
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
    dot(hash2(PT + vec2(.0, 1.)), pt-vec2(.0, 1.)), dot(hash2(PT + vec2(1., 1.)), pt-vec2(1., 1.)),
    dot(hash2(PT + vec2(.0, .0)), pt-vec2(.0, .0)), dot(hash2(PT + vec2(1., .0)), pt-vec2(1., 0.))
  );

  return 5.*mix (mix (grads.z, grads.w, mmpt.x), mix (grads.x, grads.y, mmpt.x), mmpt.y);
}

vec3 waterColorModifier()
{
  // compute ripple effect with perlin noise
  float rip = perlinNoise(vPos.xz * 3)
  + perlinNoise(vec2(vPos.xz + u_time * .25));
  vec3 rippleEffect = normalize(vec3(rip, rip, rip)) * .02;
  
  // compute view angle effect
  vec3 acuteMod = vec3(0);
  vec3 obtuseMod = vec3(0, 1, 0);
  vec3 lightDir = normalize(viewPos - vPos);
  //float angle = clamp(dot(lightDir, vNormal), 0, 1) * .2;
  float angle = abs(dot(lightDir, vNormal)) * .2;
  vec3 angleEffect = mix(acuteMod, obtuseMod, vec3(angle));
  
  return rippleEffect + angleEffect;
}

float waterAngleVisModifier()
{
  vec3 lightDir = normalize(viewPos - vPos);
  vec3 normal = normalize(vNormal);
  float angle = clamp(dot(lightDir, normal), 0, 1);
  float alpha = mix(1, 0, angle) * .08;
  return alpha;
}
// END WATER NOISE STUFF

///*
float ShadowCalculation(int cascadeIndex, vec4 fragPosLightSpace)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap[cascadeIndex], projCoords.xy).r;
  
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  
  // calculate bias (based on depth map resolution and slope)
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(lightPos - vPos);
  float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.0005);
  //bias = 0.0;
  
  // check whether current frag pos is in shadow
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap[cascadeIndex], 0);
  int samples = 1;
  for(int x = -samples; x <= samples; ++x)
  {
    for(int y = -samples; y <= samples; ++y)
    {
      float pcfDepth = texture(shadowMap[cascadeIndex], projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    }
  }
  shadow /= pow(samples * 2 + 1, 2);
  
  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if(projCoords.z > 1.0)
    shadow = 1.0;
    
  return shadow;
}
//*/

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
// returns intensity of fog, from 0 to 1
float FogCalculation()
{
  float dist = distance(vPos, viewPos);
  if (dist > fogEnd)
    return 1.0;
  if (dist < fogStart)
    return 0.0;
  return map(dist, fogStart, fogEnd, 0.0, 1.0);
}

void main()
{
  vec3 color = vColor.rgb;
  color += waterColorModifier();
  vec3 normal = normalize(vNormal);
  vec3 lightColor = dirLight.diffuse;
  
  // ambient
  vec3 ambient = dirLight.ambient * color;
  
  // diffuse
  vec3 lightDir = normalize(lightPos - vPos);
  //vec3 lightDir = -dirLight.direction;
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;
  
  // specular
  vec3 viewDir = normalize(viewPos - vPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  spec = pow(max(dot(viewDir, reflectDir), 0.0), vShininess);
  vec3 specular = spec * dirLight.specular;
  
  // calculate shadow
  float shadow = 0;
  vec3 poopoo = vec3(0);
  if (computeShadow == true)
  {
    for (int i = 0; i < NUM_CASCADES; i++)
    {
      if (ClipSpacePosZ <= cascadeEndClipSpace[i])
      {
        poopoo[i] = .2;
        shadow = ShadowCalculation(i, FragPosLightSpace[i]);
        break;
      }
    }
  }
  //float shadow = ShadowCalculation(FragPosLightSpace);                      
  vec3 lighting = (ambient + (1.05 - shadow * 1.00001) * (diffuse + specular)) * color;    
  vec4 irrelevant = vec4(lighting, 0) * 0.0001;
  
  float waterVis = waterAngleVisModifier();
  if (computeSSR == true)
    lighting += ssr() * 0.200;


  float depthDiff = distance(calcViewPosition(ssTexCoords), calcViewPositionDepthTex(ssTexCoords));
  depthDiff = clamp(depthDiff * 1.5, 0, 1);
  //depthDiff = 1 - depthDiff;
  //depthDiff *= clamp(perlinNoise(vPos.xz * 15.0 + 2.0 * u_time) / 5.0, 0, 1);
  vec3 foam = vec3(mix(1.0, clamp(perlinNoise(vPos.xz * 15.0 + 2.0 * u_time) / 5.0, 0, 1), depthDiff));
  //fragColor = vec4(mix(lighting, vec3(clamp(perlinNoise(vPos.xz * 30.0),0,1)), depthDiff / 1), vColor.a + waterVis);
  
  // TODO: lighting stuff
  float sunLight = vSunlight;
  sunLight *= max(dot(-dirLight.direction, vec3(0, 1, 0)), 0.0);
  lighting = max(lighting * .2, lighting * sunLight); // magic (ensures lighting doesn't get too dark)
  lighting = mix(lighting, fogColor, FogCalculation());
  fragColor = vec4(mix(lighting, foam, 1-depthDiff), vColor.a + waterVis);

  //fragColor = vec4(ssr(), 1.0) + fragColor * .0001;
  //fragColor = vec4(vNormal * .5 + .5, 1.0) + fragColor * ssr().rrrr * .00001;
  //fragColor = vec4(ssTexCoords, 0.0, 1.0) + (fragColor + vec4(ssr(), 0)) * .00001;
  //fragColor = vec4(normal, 1.) + irrelevant;
}