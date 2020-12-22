#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) uniform mat4 u_viewproj;
layout (location = 1) uniform mat4 u_model;

layout (location = 0) out vec2 vTexCoord;
layout (location = 1) out vec4 vColor;

struct Particle
{
    vec4 pos;
    vec4 scale;
    vec4 velocity;
    vec4 accel;
    vec4 color;
    float life;
};

layout (std430, binding = 0) readonly buffer data
{
	Particle particles[];
};

const vec2 positions[] =
{
    vec2(-.5, -.5),
    vec2(.5, .5),
    vec2(-.5, .5),
    vec2(-.5, -.5),
    vec2(.5, -.5),
    vec2(.5, .5)
};
const vec2 texcoords[] =
{
    vec2(0.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
};

void main()
{
    //vec3 aPos = vec3(positions[gl_VertexID], 0.0); // gl_VertexIndex for Vulkan
    //vec2 aTexCoord = texcoords[gl_VertexID];
    int index = gl_InstanceID;
    
    Particle particle = particles[index];
    
    vTexCoord = aTexCoord;
    vColor = particle.color;

    gl_Position = u_viewproj * u_model * vec4((aPos * particle.scale.xyz) + particle.pos.xyz, 1.0);
    //gl_Position = u_viewproj * u_model * vec4(aPos * 1.0 + particle.pos.xyz, 1.0);
}