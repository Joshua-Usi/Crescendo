#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	layout(offset = 8) uint skyboxTexIdx;
};

void main()
{
	oColor = texture(uTextures2D[skyboxTexIdx], iTexCoord);
}