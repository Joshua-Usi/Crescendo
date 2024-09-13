#version 460
#include "bindless.glsl"

layout (location = 0) in vec2 iTexCoord;
// Mapped between 1.0f and 0.0f, 1.0f meaning alive, 0.0f meaning dead
layout (location = 1) in flat float iParticleLifetime;

layout (location = 0) out vec4 oColor;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

layout(push_constant) uniform PushConstants {
	layout(offset = 20) uint diffuseTexIdx;
};

void main() {
	vec4 texelColor = texture(uTextures2D[diffuseTexIdx], iTexCoord);
	float alpha = map(iParticleLifetime, 0.0, 1.0, 0.0, 1.0);
	oColor = vec4(texelColor.rgb, texelColor.a * alpha);
}