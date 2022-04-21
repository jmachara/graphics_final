#version 330 core

layout(location=0) out vec4 color;

uniform sampler2D noise;

in vec2 texCoord;

void main()
{
	color = vec4(texture2D(noise,texCoord).xyz,1);
}