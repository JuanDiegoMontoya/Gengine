#version 460 core
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
  gl_Position = vec4(CreateTri(gl_VertexID) * 2.0, 1.0, 1.0);
}