#ifndef NOISE_H
#define NOISE_H

// random
float gold_noise(in vec2 xy, in float seed)
{
  const float PHI = 1.61803398874989484820459;
  return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
}

float silver_noise(in vec2 xy, in float seed)
{
  const float PHI = 1.61803398874989484820459;
  return fract(sin(distance(xy * PHI, xy) * seed) * xy.x);
}

// classic noise
float rand(vec2 co)
{
  return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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

#endif // NOISE_H