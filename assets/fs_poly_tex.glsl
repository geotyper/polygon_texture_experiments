#version 300 es

precision mediump float;
//precision highp float;

out vec4 fragColor;
in vec2 v_texCoord;
uniform sampler2D s_texture;

void main() {
  fragColor=texture(s_texture, v_texCoord );
}
