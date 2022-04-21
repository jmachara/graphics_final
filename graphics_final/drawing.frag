#version 330 core

layout(location=0) out vec4 color;

uniform sampler2D edgeMap;
uniform sampler2D shadeMap; // 2D texture with color values
uniform sampler2D noiseMap; // 2D texture with offset values
uniform sampler2D depthMap;

uniform vec2 dim;
in float d;

void main()
{
	vec2 texCoord = vec2(gl_FragCoord.x/dim.x,gl_FragCoord.y/dim.y);
	vec2 noiseValue = texture2D(noiseMap,texCoord).xy;
	vec2 noisytexCoord = texCoord + noiseValue/(2*d);
	float depth = texture2D(depthMap,noisytexCoord).r;
	vec3 edgeColor = texture2D(edgeMap,noisytexCoord).xyz;//*texture2D(edgeMap,noisytexCoord2).xyz;
	vec3 objColor = texture2D(shadeMap,noisytexCoord).xyz;
	bvec3 comp = equal(edgeColor,vec3(1,1,1));
	if(depth == 1.0 && comp[0] && comp[1] && comp[2]) { 
		discard; 
	}
	color = vec4(edgeColor-objColor,1);
}
