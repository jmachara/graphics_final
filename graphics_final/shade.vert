#version 330 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
layout(location=2) in vec2 txc;

uniform mat4 mvp;
uniform mat3 norm_mv;
uniform vec3 l_dir;
uniform vec3 v_dir;
uniform vec3 amb_l;
uniform highp float alpha;
uniform highp float c;

out vec2 texc;
out vec3 light_dir;
out vec3 normal;
out vec3 view_dir;
out highp float a;
out highp float c_out;
void main()
{
	gl_Position = mvp*vec4(pos,1);
	texc = txc;
	light_dir = l_dir;
	normal = normalize(norm_mv*norm);
	view_dir = v_dir;
	a = alpha;
	c_out = c;
}