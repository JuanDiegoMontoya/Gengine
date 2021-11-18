#ifndef PBR_H
#define PBR_H

#define M_PI (3.1415926536)

float D_GGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float cosTheta = max(dot(N, H), 0.0);
  float tmp = pow(cosTheta * ((a * a) - 1.0) + 1.0, 2.0);
  return (a * a) / (M_PI * tmp);
}

vec2 Hammersley(uint i, uint N)
{
  return vec2(
    float(i) / float(N),
    float(bitfieldReverse(i)) * 2.3283064365386963e-10
  );
}

vec3 ImportanceSampleGGX(vec2 xi, vec3 N, float roughness)
{
  float a = roughness * roughness;

  float phi = 2.0 * M_PI * xi.x;
  float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float G_SchlickGGX(vec3 N, vec3 I, float roughness)
{
  float k = pow(roughness + 1.0, 2.0) / 8.0;
  float cosTheta = max(dot(N, I), 0.0);
  return cosTheta / (cosTheta * (1.0 - k) + k);
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
  return G_SchlickGGX(N, V, roughness) * G_SchlickGGX(N, L, roughness);
}

#endif