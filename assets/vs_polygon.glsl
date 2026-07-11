#version 300 es

precision mediump float;

layout(location = 0) in vec2 intPosition;
layout(location = 1) in vec4 inColor;

layout (std140) uniform uBlock
{
  vec2  uWorldSize;
  vec2  uTranslate;
  float uScale;
};

out vec4  TriangleColor;

void main() {

  vec2 scaledPos=intPosition;
  vec2 posWorld=vec2(2.0*(scaledPos.x)/uWorldSize.x-1.0,2.0*(scaledPos.y)/uWorldSize.y-1.0);
  gl_Position=vec4(posWorld.x, posWorld.y, 0.1, 1.0);

  TriangleColor=inColor;
}
