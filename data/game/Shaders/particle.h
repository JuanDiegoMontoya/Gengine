#ifndef PARTICLE_H
#define PARTICLE_H

//struct Particle
//{
//  vec4 pos;
//  vec4 velocity;
//  vec4 accel;
//  vec4 color;
//  float life;
//  int alive;
//  vec2 scale;
//};

struct ParticleSharedData
{
  vec4 position;
};

struct ParticleUpdateData
{
  vec4 velocity_L;     // .w = life
  vec4 acceleration_A; // .w = alive
};

struct ParticleRenderData
{
  vec4 color; // RGBA
  vec4 scale; // XY
};

#endif // PARTICLE_H