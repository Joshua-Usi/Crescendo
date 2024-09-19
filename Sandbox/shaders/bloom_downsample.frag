#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	uint imageIdx;
    float threshold;
};

// Rec. 709 color space, standard for sRGB
float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

vec3 applyThreshold(vec3 color, float threshold) {
    return color * step(threshold, luminance(color));
}

void main()
{
	vec2 texelSize = 1.0 / textureSize(uTextures2D[imageIdx], 0);
	float x = texelSize.x;
	float y = texelSize.y;

    // Adapted and taken from LearnOpenGL
	// Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x - 2 * x, iTexCoord.y + 2 * y)).rgb, threshold);
    vec3 b = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x,         iTexCoord.y + 2 * y)).rgb, threshold);
    vec3 c = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x + 2 * x, iTexCoord.y + 2 * y)).rgb, threshold);

    vec3 d = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x - 2 * x, iTexCoord.y)).rgb, threshold);
    vec3 e = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x,         iTexCoord.y)).rgb, threshold);  // Center texel
    vec3 f = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x + 2 * x, iTexCoord.y)).rgb, threshold);

    vec3 g = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x - 2 * x, iTexCoord.y - 2 * y)).rgb, threshold);
    vec3 h = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x,         iTexCoord.y - 2 * y)).rgb, threshold);
    vec3 i = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x + 2 * x, iTexCoord.y - 2 * y)).rgb, threshold);

    vec3 j = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x - x, iTexCoord.y + y)).rgb, threshold);
    vec3 k = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x + x, iTexCoord.y + y)).rgb, threshold);
    vec3 l = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x - x, iTexCoord.y - y)).rgb, threshold);
    vec3 m = applyThreshold(texture(uTextures2D[imageIdx], vec2(iTexCoord.x + x, iTexCoord.y - y)).rgb, threshold);

    // Average the samples:
    vec3 bloom = e * 0.125;
    bloom += (a + c + g + i) * 0.03125;
    bloom += (b + d + f + h) * 0.0625;
    bloom += (j + k + l + m) * 0.125;

    // So we don't have a black
    oColor = vec4(bloom, 1.0);
}