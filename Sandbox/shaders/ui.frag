#version 460

layout(location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(set = 2, binding = 0) uniform sampler2D diffuseTex;

void main()
{
	float depth = texture(diffuseTex, iTexCoord).r;
	oColor = vec4(vec3(1.0f - depth), 1.0f);
}