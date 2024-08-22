#version 460
#include "bindless.glsl"

layout (location = 0) in vec2 iTexCoord;
layout (location = 1) in float oParticleDeathTime;

layout (location = 0) out vec4 oColor;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

layout(push_constant) uniform PushConstants {
	layout(offset = 20) float currentTime;
	uint diffuseTexIdx;
};

void main() {
	vec4 texelColor = texture(uTextures2D[diffuseTexIdx], iTexCoord);

	// alpha based on particle death time, 1s before death, alpha is 1, 0s before death, alpha is 0
	float alpha = map(oParticleDeathTime, currentTime, currentTime + 3.0, 0.0, 0.5);

	// oColor = vec4(iTexCoord, 0.0, 1.0);
	oColor = vec4(texelColor.rgb, texelColor.a * alpha);
}