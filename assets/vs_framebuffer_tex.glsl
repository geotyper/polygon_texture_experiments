#version 300 es

precision mediump float;

layout(location = 0) in vec3 intPosition;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

void main() {

//  vec2 scaledPos=uTranslate+uScale*intPosition;
//  vec2 posWorld=vec2(2.0*(scaledPos.x)/uWorldSize.x-1.0,2.0*(scaledPos.y)/uWorldSize.y-1.0);
  gl_Position=vec4(intPosition.xyz, 1.0);

  v_texCoord = a_texCoord;
}
