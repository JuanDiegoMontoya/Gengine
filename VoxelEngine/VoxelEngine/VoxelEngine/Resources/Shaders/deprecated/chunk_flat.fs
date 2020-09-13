#version 450 core


#ifdef GLSLANG
#extension GL_GOOGLE_include_directive : require
#endif

#include "TESTINCLUDE.h"

in vec4 vColor;

out vec4 fragColor;

void main()
{
  fragColor = vColor;
  fragColor = plus(fragColor);
}