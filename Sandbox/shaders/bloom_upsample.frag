#version 460
#include "bindless.glsl"

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	uint imageIdx;
};

void main()
{
    vec2 texelSize = 1.0 / textureSize(uTextures2D[imageIdx], 0);

	// const float o = max(texelSize.x, texelSize.y) * 0.01;
    const float o = 1.0 / 128.0;

    // Adapted and taken from LearnOpenGL
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
	vec3 a = texture(uTextures2D[imageIdx], vec2(iTexCoord.x - o, iTexCoord.y + o)).rgb;
    vec3 b = texture(uTextures2D[imageIdx], vec2(iTexCoord.x,     iTexCoord.y + o)).rgb;
    vec3 c = texture(uTextures2D[imageIdx], vec2(iTexCoord.x + o, iTexCoord.y + o)).rgb;

    vec3 d = texture(uTextures2D[imageIdx], vec2(iTexCoord.x - o, iTexCoord.y)).rgb;
    vec3 e = texture(uTextures2D[imageIdx], vec2(iTexCoord.x,     iTexCoord.y)).rgb;
    vec3 f = texture(uTextures2D[imageIdx], vec2(iTexCoord.x + o, iTexCoord.y)).rgb;

    vec3 g = texture(uTextures2D[imageIdx], vec2(iTexCoord.x - o, iTexCoord.y - o)).rgb;
    vec3 h = texture(uTextures2D[imageIdx], vec2(iTexCoord.x,     iTexCoord.y - o)).rgb;
    vec3 i = texture(uTextures2D[imageIdx], vec2(iTexCoord.x + o, iTexCoord.y - o)).rgb;

    vec3 avg = e * 4.0;
    avg += (b + d + f + h) * 2.0;
    avg +=  a + c + g + i;
    avg *= 1.0 / 16.0;

    oColor = vec4(avg, 1.0);
}