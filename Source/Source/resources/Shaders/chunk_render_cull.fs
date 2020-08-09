#version 460 core
#define DEBUG_VIEW 1

layout (early_fragment_tests) in;

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

layout(std430, binding = 1) writeonly buffer cmds
{
  DrawArraysCommand drawCommands[];
};

in flat int vID;

#if DEBUG_VIEW
out vec4 fragColor;
uniform bool u_debugDraw = false;
#endif

void main()
{
#if DEBUG_VIEW
  if (u_debugDraw)
    fragColor = vec4(1, 0, 1, 1);
#endif
  drawCommands[vID].instanceCount = 1;
}
