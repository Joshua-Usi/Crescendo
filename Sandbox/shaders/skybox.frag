#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform DrawParameters {
	uint camera;
	uint skyboxTexture;
	uint pad0;
	uint pad1;
};

void main()
{
	oColor = texture(uTextures2D[skyboxTexture], iTexCoord);
}