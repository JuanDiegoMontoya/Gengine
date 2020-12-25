#ifndef PARTICLE_H
#define PARTICLE_H

struct Particle
{
  vec4 pos;
  vec4 velocity;
  vec4 accel;
  vec4 color;
  float life;
  int alive;
  vec2 scale;
};

#endif // PARTICLE_H