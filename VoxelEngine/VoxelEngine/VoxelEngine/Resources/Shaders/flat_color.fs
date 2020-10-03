#version 450 core

#ifdef GLSLANG
#extension GL_GOOGLE_include_directive : require
#endif

uniform vec4 u_color;
out vec4 color;

//#include "TESTINCLUDE.h"

void main()
{
  color = u_color;
  //color = plus(color);
}