#ifndef NOISE_H
#define NOISE_H

float gold_noise(in vec2 xy, in float seed)
{
  const float PHI = 1.61803398874989484820459;  // Golden Ratio   
  return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
}

#endif // NOISE_H