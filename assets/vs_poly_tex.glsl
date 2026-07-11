#version 300 es

precision mediump float;

layout(location = 0) in vec2 intPosition;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

uniform float uZoom;
uniform float uUVZoom;
uniform float uUVAngle;

vec2 rotateUV(vec2 uv, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    mat2 r = mat2(c, -s, s, c);
    return r * (uv - vec2(0.5, 0.5)) + vec2(0.5, 0.5);
}

void main() {

//  vec2 scaledPos=uTranslate+uScale*intPosition;
//  vec2 posWorld=vec2(2.0*(scaledPos.x)/uWorldSize.x-1.0,2.0*(scaledPos.y)/uWorldSize.y-1.0);
vec2 centered = intPosition - vec2(1000.0, 1000.0);
vec2 scaledPos = centered * uZoom + vec2(1000.0, 1000.0);
vec2 posWorld=vec2(2.0*(scaledPos.x)/2000.0-1.0,2.0*(scaledPos.y)/2000.0-1.0);
gl_Position=vec4(posWorld.x, posWorld.y, 0.2, 1.0);
//  gl_Position=vec4(intPosition.xy, 0.1,1.0);

  vec2 zoomedUV = (a_texCoord - vec2(0.5, 0.5)) * uUVZoom + vec2(0.5, 0.5);
  v_texCoord = rotateUV(zoomedUV, uUVAngle);
}
