#version 330 core

layout(location=0) out vec4 color;

in vec3 normal;
in float depth;

void main()
{
	vec3 norm = vec3((normalize(normal)+1)/2);
	float depth = ((2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near)*.5)+.5;
	color = vec4(norm,depth); 
	gl_FragDepth = gl_FragCoord.z;
}