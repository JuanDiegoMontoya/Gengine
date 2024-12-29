#version 460 core
#define DEBUG_VIEW 1

#include "indirect.h.glsl"

layout (early_fragment_tests) in;

layout(std430, binding = 1) buffer cmds
{
  DrawArraysCommand drawCommands[];
};

layout(location = 0) in flat int vID;

#if DEBUG_VIEW
layout(location = 0) out vec4 fragColor;
layout(location = 3) uniform bool u_debugDraw = false;
#endif

void main()
{
#if DEBUG_VIEW
  if (u_debugDraw)
    fragColor = vec4(1, float(vID) / 2000000000.f, 1, 1);
#endif

  drawCommands[vID].instanceCount = 1;
}
