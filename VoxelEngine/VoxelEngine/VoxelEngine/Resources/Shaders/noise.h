#ifndef NOISE_H
#define NOISE_H

#if 1
float gold_noise(in vec2 xy, in float seed)
{
  const float PHI = 1.61803398874989484820459;  // Golden Ratio   
  return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
}
#else
float gold_noise(vec2 co, in float seed)
{
  return fract(sin(dot(co.xy + vec2(seed), vec2(12.9898,78.233))) * 43758.5453);
}
#endif
#endif // NOISE_H