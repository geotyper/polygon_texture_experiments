#version 300 es

precision mediump float;

layout(location = 0) in vec2  inPos;

// Instanced attributes
layout(location = 1) in vec2  instancePos;
layout(location = 2) in float instanceRot;
layout(location = 3) in float instanceScale;
//layout(location = 3) in vec4  inColor;

layout (std140) uniform uBlock
{
  vec2  uWorldSize;
  vec2  uTranslate;
  float uScale;
};

out vec4   TriangleColor;

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

void main() {

  //int instId=gl_InstanceIndex;

	vec2 scaledPos=vec2(instanceScale*inPos.x,instanceScale*inPos.y);
	scaledPos=rotate(scaledPos, instanceRot);
	vec3 pos = vec3(2.0*(scaledPos.x+instancePos.x)/uWorldSize.x-1.0,2.0*(scaledPos.y+instancePos.y)/uWorldSize.y-1.0,0.0);

  gl_Position = vec4(pos, 1.0f);

  TriangleColor=vec4(0.2,0.75,0.75,1.0);//inColor;
}
