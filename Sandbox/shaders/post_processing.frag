#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	uint offscreenIdx;
	uint bloomIdx;
};

vec3 ACESFilm(vec3 x) {
	// a and c are determine the strength of the tonemap. Higher values result in higher constrast
	// b and e define offsets that manage the black level of toe of the curve, controls how the darkest parts are rendered
	// d is the midtone coefficient, controling midtones of the image
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	vec3 hdrColor = texture(uTextures2D[offscreenIdx], iTexCoord).rgb;
	vec3 bloomColor = texture(uTextures2D[bloomIdx], iTexCoord).rgb;

	// Tonemapped with ACES
	oColor = vec4(ACESFilm(hdrColor), 1.0);

	// Bloom
	oColor.rgb = oColor.rgb + bloomColor;
}