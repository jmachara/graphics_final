#version 330 core

layout(location=0) out vec4 color;

uniform sampler2D tex;

in vec2 texc;
in vec3 light_dir;
in vec3 normal;
in vec3 view_dir;
in highp float a;
in highp float c_out;


void main()
{
	vec4 clr = vec4(1,0,0,1);
	if(c_out > .1)
	{
		 clr = vec4(texture2D(tex,texc).xyz,1);
	}
	highp float kd_ang = dot(normal,light_dir);
	vec4 ks = vec4(1,1,1,1);
	vec3 hlf_ang = normalize(light_dir+view_dir);
	vec4 spec = ks*pow(dot(normalize(normal),hlf_ang),a);
	color = spec+clr;
}