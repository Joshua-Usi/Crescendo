#version 460
layout(location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform sampler2D diffuse;

void main()
{
	oColor = texture(diffuse, iTexCoord);
}