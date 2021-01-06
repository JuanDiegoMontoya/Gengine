#version 460 core

layout (location = 0) out vec2 vTexCoord;

// (0, 2) 01
// (0, 0) 00
// (2, 0) 10
// ccw tri in [0, 1]
vec2 CreateTri(uint vertexID) // GL_TRIANGLES
{
  uint b = 1 << vertexID;
  return vec2((0x1 & b) != 0, (0x4 & b) != 0);
}

void main()
{
  vec2 pos = CreateTri(gl_VertexID); 
  vTexCoord = pos * 2.0;
  gl_Position = vec4(pos * 4.0 - 1.0, 1.0, 1.0); // xy in [-1, 3]
}