#version 450 core

//layout (location = 0) out vec4 FragColor;
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform bool sharpenFilter;
uniform bool blurFilter;
uniform bool edgeDetection;
uniform bool chromaticAberration;

#define ID2D(x, y, w) (width * row + col)
const float offset = 1.0 / 1000.0;


uniform float near_plane = .1;
uniform float far_plane = 100;
uniform float fogStart = 1.5;
uniform float fogEnd = 2;
uniform vec3 fogColor;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}


float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}


float FogCalculation()
{
  //float dist = distance(vPos, viewPos);
  float dist = LinearizeDepth(texture(depthTex, TexCoords).r);
  dist = clamp(dist, 0, 1);
  return map(dist, fogStart, fogEnd, 0.0, 1.0);
}


const vec2 offsets3x3[9] = vec2[](
  vec2(-offset,  offset), // top-left
  vec2( 0.0f,    offset), // top-center
  vec2( offset,  offset), // top-right
  vec2(-offset,  0.0f),   // center-left
  vec2( 0.0f,    0.0f),   // center-center
  vec2( offset,  0.0f),   // center-right
  vec2(-offset, -offset), // bottom-left
  vec2( 0.0f,   -offset), // bottom-center
  vec2( offset, -offset)  // bottom-right    
);


void calc3x3kernel(inout vec3 rgb, in float kernel[9])
{
  vec3 sampleTex[9];
  for(int i = 0; i < 9; i++)
    sampleTex[i] = texture(colorTex, TexCoords.xy + offsets3x3[i]).rgb;
  for(int i = 0; i < 9; i++)
    rgb += sampleTex[i] * kernel[i];
}


void main()
{
  int ppEffectCount = 0; // divisor to final color to combine effects properly
  vec3 rgb = vec3(0.0);
  
  if (sharpenFilter == true)
  {
    float kernel[9] = float[](
      -1, -1, -1,
      -1,  9, -1,
      -1, -1, -1
    );
    calc3x3kernel(rgb, kernel);
    ppEffectCount++;
  }

  if (edgeDetection == true)
  {
    float kernel[9] = float[](
      1,  1, 1,
      1, -8, 1,
      1,  1, 1
    );
    calc3x3kernel(rgb, kernel);
    ppEffectCount++;
  }

  if (chromaticAberration == true)
  {
    vec3 colorOffsets = vec3(offset, -offset, 0);
    rgb.r += texture(colorTex, TexCoords.xy + colorOffsets.r).r;
    rgb.g += texture(colorTex, TexCoords.xy + colorOffsets.g).g;
    rgb.b += texture(colorTex, TexCoords.xy + colorOffsets.b).b;
    ppEffectCount++;
  }

  if (blurFilter == true)
  {
    float kernel[9] = float[](
      1.0 / 16, 2.0 / 16, 1.0 / 16,
      2.0 / 16, 4.0 / 16, 2.0 / 16,
      1.0 / 16, 2.0 / 16, 1.0 / 16  
    );
    calc3x3kernel(rgb, kernel);
    ppEffectCount++;
  }

  if (ppEffectCount == 0)
  {
    ppEffectCount = 1;
    rgb = texture(colorTex, TexCoords.xy).rgb;
  }

  //rgb = mix(rgb, fogColor, FogCalculation());
  FragColor = vec4(rgb / ppEffectCount, 1);
  //FragColor.rgb = vec3(LinearizeDepth(texture(depthTex, TexCoords.xy).r)) - 1.711 * FragColor.rgb;
}