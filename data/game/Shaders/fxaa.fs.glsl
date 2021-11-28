#version 460 core

//#define QUALITY_PRECISE
#define QUALITY_HIGH
//#define QUALITY_LOW

#if defined(QUALITY_PRECISE)
  #define EDGE_STEP_COUNT 10
  #define EDGE_STEPS 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
  #define EDGE_GUESS 1.0
#elif defined(QUALITY_HIGH)
  #define EDGE_STEP_COUNT 10
  #define EDGE_STEPS 1, 1.5, 2, 2, 2, 2, 2, 2, 2, 4
  #define EDGE_GUESS 8
#elif defined(QUALITY_LOW)
  #define EDGE_STEP_COUNT 4
  #define EDGE_STEPS 1, 1.5, 2, 4
  #define EDGE_GUESS 12
#endif

layout (location = 0) in vec2 vTexCoord;

layout (binding = 0) uniform sampler2D colorTex;
layout (location = 1) uniform vec2 u_invScreenSize;
layout (location = 2) uniform float u_contrastThreshold = .0312; // .0833, .0625, .0312 from lowest to best quality
layout (location = 3) uniform float u_relativeThreshold = .125; // .250, .166, .125
layout (location = 4) uniform float u_pixelBlendStrength = 1.0;
layout (location = 5) uniform float u_edgeBlendStrength = 1.0;

layout (location = 0) out vec4 fragColor;

float ColorToLum(vec3 c)
{
  //return sqrt(dot(c, vec3(0.299, 0.587, 0.114)));
  return dot(c, vec3(0.213,  0.715, 0.072)); // sRGB luminance calculation
}

const float edgeSteps[EDGE_STEP_COUNT] = { EDGE_STEPS };

vec3 ComputeFXAA()
{
  // get immediate neighborhood
  vec3 colorCenter = textureOffset(colorTex, vTexCoord, ivec2(0, 0)).rgb;
  vec3 colorN = textureOffset(colorTex, vTexCoord, ivec2(0, 1)).rgb;
  vec3 colorS = textureOffset(colorTex, vTexCoord, ivec2(0, -1)).rgb;
  vec3 colorW = textureOffset(colorTex, vTexCoord, ivec2(-1, 0)).rgb;
  vec3 colorE = textureOffset(colorTex, vTexCoord, ivec2(1, 0)).rgb;
  float lumCenter = ColorToLum(colorCenter);
  float lumN = ColorToLum(colorN);
  float lumS = ColorToLum(colorS);
  float lumW = ColorToLum(colorW);
  float lumE = ColorToLum(colorE);

  float lumMax = max(max(max(max(lumCenter, lumN), lumS), lumW), lumE);
  float lumMin = min(min(min(min(lumCenter, lumN), lumS), lumW), lumE);
  float lumContrast = lumMax - lumMin;

  // early exit for fragments in low contrast areas
  float threshold = max(u_contrastThreshold, u_relativeThreshold * lumMax);
  if (lumContrast < threshold)
  {
    return colorCenter;
  }

  // get diagonal neighborhood
  vec3 colorNW = textureOffset(colorTex, vTexCoord, ivec2(-1, 1)).rgb;
  vec3 colorNE = textureOffset(colorTex, vTexCoord, ivec2(1, 1)).rgb;
  vec3 colorSW = textureOffset(colorTex, vTexCoord, ivec2(-1, -1)).rgb;
  vec3 colorSE = textureOffset(colorTex, vTexCoord, ivec2(1, -1)).rgb;
  float lumNW = ColorToLum(colorNW);
  float lumNE = ColorToLum(colorNE);
  float lumSW = ColorToLum(colorSW);
  float lumSE = ColorToLum(colorSE);

  // double weight given to immediate neighborhood, single weight given to corners
  float pixelBlend = 2.0 * (lumN + lumS + lumW + lumE) + (lumNW + lumNE + lumSW + lumSE);
  pixelBlend /= 12.0; // divide by sum of weights
  pixelBlend = abs(pixelBlend - lumCenter);
  pixelBlend = clamp(pixelBlend / lumContrast, 0.0, 1.0);
  pixelBlend = smoothstep(0.0, 1.0, pixelBlend);
  pixelBlend *= pixelBlend;
  pixelBlend *= u_pixelBlendStrength;

  // determine direction to blend
  float horizontalGrad = 
    abs(lumN + lumS - 2.0 * lumCenter) * 2.0 +
    abs(lumNE + lumSE - 2.0 * lumE) +
    abs(lumNW + lumSW - 2.0 * lumW);
  float verticalGrad = 
    abs(lumE + lumW - 2.0 * lumCenter) * 2.0 +
    abs(lumNE + lumNW - 2.0 * lumN) + 
    abs(lumSE + lumSW - 2.0 * lumS);
  bool isEdgeHorizontal = horizontalGrad >= verticalGrad;

  float posLum = isEdgeHorizontal ? lumN : lumE;
  float negLum = isEdgeHorizontal ? lumS : lumW;
  float posGrad = abs(posLum - lumCenter);
  float negGrad = abs(negLum - lumCenter);

  float pixelStep = isEdgeHorizontal ? u_invScreenSize.y : u_invScreenSize.x;
  float oppLum;
  float gradient;
  if (posGrad < negGrad)
  {
    pixelStep = -pixelStep;
    oppLum = negLum;
    gradient = negGrad;
  }
  else
  {
    oppLum = posLum;
    gradient = posGrad;
  }
  
  vec2 uvEdge = vTexCoord;
  vec2 edgeStep;
  if (isEdgeHorizontal)
  {
    uvEdge.y += pixelStep * 0.5;
    edgeStep = vec2(u_invScreenSize.x, 0.0);
  }
  else
  {
    uvEdge.x += pixelStep * 0.5;
    edgeStep = vec2(0.0, u_invScreenSize.y);
  }

  float edgeLum = (lumCenter + oppLum) / 2.0;
  float gradientThreshold = gradient / 4.0;

  // positive edge stepping
  vec2 posUV = uvEdge + edgeStep * edgeSteps[0];
  float posLumDelta = ColorToLum(texture(colorTex, posUV).rgb) - edgeLum;
  bool posAtEnd = abs(posLumDelta) >= gradientThreshold;
  for (uint i = 0; i < EDGE_STEP_COUNT && !posAtEnd; i++)
  {
    posUV += edgeStep * edgeSteps[i];
    posLumDelta = ColorToLum(texture(colorTex, posUV).rgb) - edgeLum;
    posAtEnd = abs(posLumDelta) >= gradientThreshold;
  }
  if (!posAtEnd)
  {
    posUV += edgeStep * EDGE_GUESS;
  }

  // negative edge stepping
  vec2 negUV = uvEdge - edgeStep * edgeSteps[0];
  float negLumDelta = ColorToLum(texture(colorTex, negUV).rgb) - edgeLum;
  bool negAtEnd = abs(negLumDelta) >= gradientThreshold;
  for (uint i = 0; i < EDGE_STEP_COUNT && !negAtEnd; i++)
  {
    negUV -= edgeStep * edgeSteps[i];
    negLumDelta = ColorToLum(texture(colorTex, negUV).rgb) - edgeLum;
    negAtEnd = abs(negLumDelta) >= gradientThreshold;
  }
  if (!negAtEnd)
  {
    negUV -= edgeStep * EDGE_GUESS;
  }

  float posDist;
  float negDist;
  if (isEdgeHorizontal)
  {
    posDist = posUV.x - vTexCoord.x;
    negDist = vTexCoord.x - negUV.x;
  }
  else
  {
    posDist = posUV.y - vTexCoord.y;
    negDist = vTexCoord.y - negUV.y;
  }

  float shortestDist;
  bool deltaSign;
  if (posDist <= negDist)
  {
    shortestDist = posDist;
    deltaSign = posLumDelta >= 0.0;
  }
  else
  {
    shortestDist = negDist;
    deltaSign = negLumDelta >= 0.0;
  }

  float edgeBlend;
  if (deltaSign == (lumCenter - edgeLum >= 0.0))
  {
    edgeBlend = 0;
  }
  else
  {
    edgeBlend = 0.5 - shortestDist / (posDist + negDist);
    edgeBlend *= u_edgeBlendStrength;
  }

  float finalBlendFactor = max(pixelBlend, edgeBlend);

  vec2 offset;
  if (isEdgeHorizontal)
  {
    offset = vec2(0.0, pixelStep * finalBlendFactor);
  }
  else
  {
    offset = vec2(pixelStep * finalBlendFactor, 0.0);
  }
  vec2 uv = vTexCoord + offset;

  return textureLod(colorTex, uv, 0).rgb;
  //return textureLod(colorTex, uv, 0).rgb * .0001 + vec3(finalBlendFactor);
}

void main()
{
  fragColor = vec4(ComputeFXAA(), 1.0);
}