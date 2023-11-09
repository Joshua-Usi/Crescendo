#version 460
layout (location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(set = 2, binding = 0) uniform sampler2D diffuseTex;

void main()
{
	oColor = texture(diffuseTex, iTexCoord);
}