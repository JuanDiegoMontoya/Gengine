#version 460 core

// aEncoded layout (left to right bits):
// 0 - 17   18 - 20   21 - 31
// vertex   normal    texcoord
// vertex = x, y, z from 0-32 (supports up to 63)
// normal = 0 - 5 index into "normals" table
// texcoord = texture index (0 - 512), corner index (0 - 3)
//layout (location = 0) in uint aEncoded; // (per vertex)

// aLighting layout
// 0 - 12   13 - 15   16 - 19   20 - 23   24 - 27   28 - 31
// unused    dirCent     R         G         B         Sun
//layout (location = 1) in uint aLighting;// (per vertex)

// per-chunk info
layout(location = 0) in ivec3 u_pos; // (per instance)

// global info
layout(location = 0) uniform mat4 u_viewProj;

// How is this buffer laid out?
//
// uint 1 holds the following info per quad
// 0 - 14              15 - 17      18 - 27        28 - 31
// block position      face         material ID    unused(4)
// block position = x, y, z in [0, 31] (15 bits)
// face = face index in [0, 5] (3 bits)
// material ID = currently, texture array index. Later, material array index (10 bits)
//
// Vertex position is derived from block position, face, and VS built-in inputs.
// UV is derived from face and built-in inputs.
// Normal is derived from face or reconstructed in fragment shader with partial derivatives.
//
// uint 2 holds the following info per quad
// 0 - 15        16 - 23     24 - 31
// lighting      quad AO     unused(8)
// lighting = RGB+Sun values, each in [0, 15] (16 bits)
// quad AO = the AO values for all four vertices of the tri, each in [0, 3] (2 bits each, 8 bits total)
layout(std430, binding = 0) restrict readonly buffer VertexData
{
  uvec2 quads[];
};

layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 vTexCoord;
layout(location = 2) out vec4 vLighting; // RGBSun
layout(location = 3) out flat uint vQuadAO;

const vec3 normals[] =
{
  { 0, 0, 1 }, // 'far' face    (+z direction)
  { 0, 0,-1 }, // 'near' face   (-z direction)
  {-1, 0, 0 }, // 'left' face   (-x direction)
  { 1, 0, 0 }, // 'right' face  (+x direction)
  { 0, 1, 0 }, // 'top' face    (+y direction)
  { 0,-1, 0 }  // 'bottom' face (-y direction)
};


// counterclockwise from bottom right texture coordinates
const vec2 tex_corners[] =
{
  { 1, 0 },
  { 1, 1 },
  { 0, 1 },
  { 0, 0 },
};


const uint indices[6] = { 0, 1, 3, 3, 1, 2 };


// decodes block position, face, and material from encoded data
void DecodeQuad(in uint encoded, out vec3 blockPos, out uint face, out uint texIdx)
{
  // decode block position
  blockPos.x = encoded & 0x1F;
  blockPos.y = (encoded >> 5) & 0x1F;
  blockPos.z = (encoded >> 10) & 0x1F;
  //modelPos += 0.5;

  // decode normal
  face = (encoded >> 15) & 0x7; // = 0b111

  // decode texture index and UV
  texIdx = (encoded >> 18) & 0x3FF; // = 0b1111111111 (10 bits)
}


// decodes the quad's lighting information into a usable vec4
void DecodeQuadLight(in uint encoded, out vec4 lighting, out uint quadAO)
{
  quadAO = (encoded >> 16) & 0xFF;

  lighting.r = (encoded >> 12) & 0xF;
  lighting.g = (encoded >> 8) & 0xF;
  lighting.b = (encoded >> 4) & 0xF;
  lighting.a = encoded & 0xF;
  lighting = lighting / 15.0;
}

vec3 ObjSpaceVertexPos(uint index, uint face)
{
  uint b = 1 << (index + (4 * face));
  return vec3((b & 0xCCF0C3) != 0, (b & 0xF6666) != 0, (b & 0x96C30F) != 0) - 0.5;
}

void main()
{
  uvec2 quadData = quads[gl_VertexID / 6];
  uint vertexIndex = indices[gl_VertexID % 6];

  // decode general
  vec3 blockPos;
  uint face;
  uint texIdx;
  DecodeQuad(quadData[0], blockPos, face, texIdx);
  vec3 blockPosWorldSpace = blockPos + u_pos + 0.5;
  vPos = blockPosWorldSpace + ObjSpaceVertexPos(vertexIndex, face);
  vTexCoord = vec3(tex_corners[vertexIndex], texIdx);

  // decode lighting + quad AO
  uint quadAO;
  DecodeQuadLight(quadData[1], vLighting, quadAO);

  vQuadAO = quadAO;
  //uint myAO = (quadAO >> (vertexIndex * 2)) & 0x3;
  //vAmbientOcclusion = float(myAO) / 3.0;

  gl_Position = u_viewProj * vec4(vPos, 1.0);
}