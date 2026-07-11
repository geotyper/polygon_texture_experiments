#version 300 es

precision mediump float;
//precision highp float;

out vec4 fragColor;
in vec4  TriangleColor;
//uniform vec4 uColor;

void main() {
  fragColor=TriangleColor;//vec4(0.3,0.7,0.9,1.0);
}
