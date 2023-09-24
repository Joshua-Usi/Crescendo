#version 450

layout(location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform data {
	float time;
};

layout(set = 2, binding = 0) uniform sampler2D albedo;

void main()
{
	oColor = texture(albedo, iTexCoord);
}