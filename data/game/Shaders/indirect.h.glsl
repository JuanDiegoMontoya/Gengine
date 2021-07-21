#ifndef INDIRECT_H
#define INDIRECT_H

struct DrawElementsCommand
{
  uint count;        // num indices in draw call
  uint instanceCount;// num instances in draw call
  uint firstIndex;   // offset in index buffer: sizeof(element)*firstIndex from start of buffer
  uint baseVertex;   // offset in vertex buffer: sizeof(vertex)*baseVertex from start of buffer
  uint baseInstance; // first instance to draw (position in instanced buffer)
};

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first; // baseVertex
  uint baseInstance;
};

#endif // INDIRECT_H