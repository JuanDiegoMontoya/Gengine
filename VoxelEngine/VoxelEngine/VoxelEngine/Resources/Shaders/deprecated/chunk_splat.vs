#version 450 core

// model pos(6 * 3 bits) + color (4 * 3 bits)
layout (location = 0) in uint aEncoded;
layout (location = 1) in ivec3 u_pos;

uniform mat4 u_viewProj;
uniform vec2 u_viewportSize;

out vec3 vPos;
out vec3 vColor;


// returns random vec3 with values in (-1, 1)
vec3 random3(vec3 c)
{
  float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
  vec3 r;
  r.z = fract(512.0*j);
  j *= .125;
  r.x = fract(512.0*j);
  j *= .125;
  r.y = fract(512.0*j);
  return r-0.5;
}


void Decode(in uint encoded, out vec3 modelPos, out vec3 color)
{
  // decode vertex position
  modelPos.x = encoded >> 26;
  modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
  modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111

  color.r = (encoded >> 10) & 0xF;
  color.g = (encoded >> 6) & 0xF;
  color.b = (encoded >> 2) & 0xF;
  color /= (1 << 4);
}


void SetPointSizeFancy(vec4 glPos)
{
  float aspectRatio = u_viewportSize.x / u_viewportSize.y;
  // embiggen point if it's near screen edge
  float reduce = max(
    abs(glPos.x * aspectRatio / glPos.w),
    abs(glPos.y / glPos.w)
  );
  reduce -= 0.13; // affects overdraw near edges
  float size = (u_viewportSize.y * 1.1) / glPos.z * max(reduce, 1.0);
  gl_PointSize = size * 0.8 + 3.0;
}


void main()
{
  vec3 modelPos;
  Decode(aEncoded, modelPos, vColor);
  vPos = modelPos + u_pos + 0.5;
  vPos += random3(vPos) / 10.0; // random jitter in (-.1, .1)
  gl_Position = u_viewProj * vec4(vPos, 1.0);

  //gl_PointSize = 2400.0 / gl_Position.z;
  SetPointSizeFancy(gl_Position);
  //gl_PointSize = 3;
  //gl_PointSize = max(100.0 / pow(gl_Position.z, .45), 4.0);

  gl_PointSize = max(gl_PointSize, 4);
  gl_PointSize = 1;
}