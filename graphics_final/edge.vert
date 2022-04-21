#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;
layout(location=3) in vec3 noise;


uniform mat4 mvp;
uniform mat3 norm_mv;

out mat4 mat;
out vec3 normal;
out vec2 texCoord;

void main()
{
	gl_Position = mvp*vec4(pos,1);
	normal = norm_mv*norm;
	mat = mvp;
	texCoord = txc;
}