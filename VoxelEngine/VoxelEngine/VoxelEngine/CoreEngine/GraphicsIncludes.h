#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <CoreEngine/MathIncludes.h>

#include <CoreEngine/utilities.h>
#include <CoreEngine/shader.h>
#include <CoreEngine/Vertices.h>
#include <CoreEngine/UBO.h>
#include <CoreEngine/StaticBuffer.h>
#include <CoreEngine/vao.h>

// note: baseInstance is for glMultiDraw*Indirect ONLY
// for any other purpose it must be zero

struct DrawElementsIndirectCommand
{
  GLuint  count;
  GLuint  instanceCount;
  GLuint  firstIndex;
  GLuint  baseVertex;
  GLuint  baseInstance;
};

struct DrawArraysIndirectCommand
{
  GLuint  count;
  GLuint  instanceCount;
  GLuint  first;
  GLuint  baseInstance;
};