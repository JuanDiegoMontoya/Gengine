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
  vec4 position_A; // .w = alive
};

struct ParticleUpdateData
{
  //vec4 velocity_L; // .w = life
  //vec4 acceleration;

  // unpackHalf2x16 x 3 to access vel/accel, uintBitsToFloat(.w) to access L
  uvec4 velocity_acceleration_L;
};

struct ParticleRenderData
{
  //vec4 color; // RGBA
  //vec4 scale; // XY

  // unpackHalf2x16, unpackUnorm4x8 to access
  uvec2 packedScaleX_packedColorY;
};

#endif // PARTICLE_H