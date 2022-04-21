#version 330 core

layout(location=0) out vec4 color;

uniform sampler2D edgeMap;
uniform sampler2D shadeMap; // 2D texture with color values
uniform sampler2D noiseMap; // 2D texture with offset values
uniform sampler2D depthMap;

uniform vec2 dim;
uniform mat2 edgeMat;
uniform mat2 shadeMat;

in float d;

void main()
{
	vec2 texCoord = vec2(gl_FragCoord.x/dim.x,gl_FragCoord.y/dim.y);
	vec2 noiseValue = texture2D(noiseMap,texCoord).xy;
	//vec2 noisytexCoord = texCoord + noiseValue/(2*d);
	vec2 edgeTexCoord = texCoord.xy + (edgeMat*noiseValue);
	vec2 shadeTexCoord = texCoord.xy + (shadeMat*noiseValue);
	//float depth = texture2D(depthMap,noisytexCoord).r;
	float edgeDepth = float(texture2D(depthMap, edgeTexCoord).xyz); 
	float shadeDepth = float(texture2D(depthMap, shadeTexCoord).xyz); 
	float depth = min(edgeDepth, shadeDepth);
	if(depth == 1.0) { 
		discard; 
	}
	gl_FragDepth = depth;
	vec4 edgeValue = texture2D(edgeMap, edgeTexCoord); 
	if(edgeDepth == 1.0) { 
		edgeValue = vec4(1.0,1.0,1.0,1.0); // white background
	} 
	vec4 shadeValue = texture2D(shadeMap, shadeTexCoord); 
	if(shadeDepth == 1.0) { 
		shadeValue = vec4(1.0,1.0,1.0,1.0); // white background
	}
	color = edgeValue*shadeValue;
}
