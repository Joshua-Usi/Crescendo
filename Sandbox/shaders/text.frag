#version 460
#include "bindless.glsl"

layout (location = 0) in vec2 iTexCoord;
layout (location = 1) in flat uint iTextureID;

layout (location = 0) out vec4 oColor;

layout (push_constant) uniform PushConstants {
	layout(offset = 36) uint color; // packed 8-bit RGBA
};

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec4 color = unpackUnorm4x8(color);
	vec3 msdf = texture(uTextures2D[iTextureID], iTexCoord * 0.95 + 0.025).rgb;

	float dist = median(msdf.r, msdf.g, msdf.b) - 0.5;
	float opacity = clamp(dist/fwidth(dist) + 0.5, 0.0, 1.0);
	oColor = vec4(color.rgb, opacity * color.a);
	
	// For debugging
	// oColor = vec4(msdf, 1.0);
}