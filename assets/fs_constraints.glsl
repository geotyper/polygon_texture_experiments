#version 300 es

precision mediump float;
//precision highp float;

out vec4 fragColor;
in vec4  TriangleColor;
uniform vec3 uConstraintColor;

void main() {
  fragColor=vec4(uConstraintColor, 1.0);
}
