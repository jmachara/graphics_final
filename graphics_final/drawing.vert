#version 330 core

layout(location=4) in vec3 pos;
layout(location=2) in vec2 txc;

uniform mat4 mvp;
uniform float dist;

out float d;


void main()
{
	gl_Position = mvp*vec4(pos,1);
	d= dist;
}