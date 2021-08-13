#ifndef PARTICLE_H
#define PARTICLE_H

// struct Particle
// {
//  vec4 position;
//  vec4 velocity;
//  vec4 acceleration;
//  vec4 color;
//  vec2 scale;
//  float life;
//  int alive;
// };

// struct Particle2
// {
//   vec4 position;
//   uvec4 velocity_acceleration_life;
//   uvec2 packedScale_packedColor;
// };

struct ParticleSharedData
{
  vec4 position;
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