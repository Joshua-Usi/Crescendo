#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	uint imageIndex;
};

vec3 ACESFilm(vec3 x) {
	// a and c are determine the strength of the tonemap. Higher values result in higher constrast
	// b and e define offsets that manage the black level of toe of the curve, controls how the darkest parts are rendered
	// d is the midtone coefficient, controling midtones of the image
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
}

void main()
{
	vec3 hdrColor = texture(uTextures2D[imageIndex], iTexCoord).rgb;

	// Tonemapped with ACES
	oColor = vec4(ACESFilm(hdrColor), 1.0f);

	// Gamma correction
	// oColor = pow(oColor, vec4(2.2f));

	// Unmodified
	// oColor = vec4(texture(image, iTexCoord).rgb, 1.0f);
	
	// Inverted
	// oColor = vec4(1.0f - pow(texture(image, iTexCoord).rgb, vec3(1.0f / 2.2f)), 1.0f);

	// Half unmodified, half inverted
	// vec3 color = texture(image, iTexCoord).rgb;
	// if (iTexCoord.x < 0.5f) oColor = vec4(color, 1.0f);
	// else oColor = vec4(1.0f - pow(color, vec3(1.0f / 2.2f)), 1.0f);

	// Grayscale
	// vec4 col = texture(image, iTexCoord);
    // float avg = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
	// oColor = vec4(avg, avg, avg, 1.0);

	// Vignette
	// const float vignetteStrength = 0.5f;
    // float dist = distance(iTexCoord, vec2(0.5f, 0.5f));
    // float vignette = smoothstep(0.45f, 0.5f * vignetteStrength, dist);
    // vec3 color = texture(image, iTexCoord).rgb;
    // oColor = vec4(mix(color, color * vignette, vignetteStrength), 1.0f);
}