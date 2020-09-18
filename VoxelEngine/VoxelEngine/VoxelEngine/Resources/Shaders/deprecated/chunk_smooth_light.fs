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
in lowp vec4 vLighting;
in vec3 vPos;
in vec4 FragPosLightSpace[NUM_CASCADES];
in float ClipSpacePosZ;

uniform sampler2D shadowMap[NUM_CASCADES];
uniform float cascadeEndClipSpace[NUM_CASCADES];
uniform DirLight dirLight; // the sun
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform bool computeShadow;

uniform float fogStart; // world
uniform float fogEnd;   // world
uniform vec3 fogColor;

out vec4 fragColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

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

// blinn-phong + cascaded shadows
void main()
{
  vec3 color = vColor.rgb;
  vec3 normal = normalize(vNormal);
  vec3 lightColor = dirLight.diffuse;
  
  // ambient
  vec3 ambient = dirLight.ambient * color;
  
  // diffuse
  vec3 lightDir = normalize(lightPos - vPos);
  //vec3 lightDir = -dirLight.direction;
  float diff = max(dot(lightDir, normal), 0.0); // 0
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

  vec3 lighting = (ambient + (1.05 - shadow * 1.00) * (diffuse + specular)) * color;
  vec4 irrelevant = vec4(lighting, 0) * 0.0001;
  
  //fragColor = vec4(poopoo, 1) + irrelevant;

  // TODO: lighting stuff (i'm tired rn)
  float sunLight = vSunlight;
  sunLight *= max(dot(-dirLight.direction, vec3(0, 1, 0)), 0.0);
  lighting = max(lighting * .2, lighting * sunLight); // magic (ensures lighting doesn't get too dark)
  lighting = mix(lighting, fogColor, FogCalculation());
  fragColor = vec4(lighting, vColor.a);
  //fragColor = vec4(vPos, 1) +  irrelevant;
  //fragColor = vec4(vec3(FragPosLightSpace[0].y / 10), 1) + irrelevant;
  //fragColor = vec4(vec3(ClipSpacePosZ) / 100, 1) + irrelevant;
  //fragColor = vec4(vec3(shadow / 3) + vec3(ClipSpacePosZ / 200) + poopoo, 1) + irrelevant;
}