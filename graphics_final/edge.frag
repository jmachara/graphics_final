#version 330 core

layout(location=0) out vec4 color;

uniform sampler2D gBuffers;

uniform vec2 texOff;
uniform vec2 dim;

in vec3 normal;
in mat4 mat;


void main()
{
	// Center and offset 
	vec2 ctr = vec2(gl_FragCoord.x/dim.x,gl_FragCoord.y/dim.y); 
	vec2 off = vec2((1.0/dim.x)*texOff.x,(1.0/dim.y)*texOff.y); 

	// Access in direction A 
	 vec2 tex = vec2(ctr.x-off.x,ctr.y+off.y);
	 vec4 A = texture2D(gBuffers, tex); 
	 A.xyz = normalize((A.xyz*2.0)-1.0); 
	 // Access in direction C 
	 tex = vec2(ctr.x+off.x, ctr.y+off.y);
	 vec4 C = texture2D(gBuffers, tex); 
	 C.xyz = normalize((C.xyz*2.0)-1.0); 
	 // Access in direction F 
	 tex = vec2(ctr.x-off.x, ctr.y-off.y);
	 vec4 F = texture2D(gBuffers, tex); 
	 F.xyz = normalize((F.xyz*2.0)-1.0); 
	 // Access in direction H 
	 tex = vec2(ctr.x+off.x, ctr.y-off.y);
	 vec4 H = texture2D(gBuffers, tex); 
	 H.xyz = normalize((H.xyz*2.0)-1.0); 
	 
	 // Calculate discontinuities 
	vec3 discontinuity = vec3(0.0, 0.0, 0.0); 
	 discontinuity.x = 0.5 * (dot(A.xyz, H.xyz) + dot(C.xyz, F.xyz)); 
	discontinuity.y = (1.0-0.5*abs(A.w-H.w)) * (1.0-0.5*abs(C.w-F.w)); 
	discontinuity.z = discontinuity.x*discontinuity.y; 
	// Output edge intensities to the edge map 
	if(discontinuity.x < .99 && discontinuity.z < .97 && gl_FrontFacing)
	{
		color = vec4(0,0,0,1);
	}
	else
	{
		color = vec4(1,1,1,1);
		//discard;
	}
}