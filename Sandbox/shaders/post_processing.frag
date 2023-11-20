#version 460

layout(location = 0) in vec2 iTexCoord;

layout(location = 0) out vec4 oColor;

layout(set = 0, binding = 0) uniform sampler2D image;

void main()
{
	// Unmodified
	// oColor = vec4(texture(image, iTexCoord).rgb, 1.0f);
	
	// Inverted
	// oColor = vec4(1.0f - texture(image, iTexCoord).rgb, 1.0f);

	// Get the texture color
	vec3 color = texture(image, iTexCoord).rgb;
	if (iTexCoord.x < 0.5f) oColor = vec4(color, 1.0f);
	else oColor = vec4(1.0f - pow(color, vec3(1.0f / 2.2f)), 1.0f);

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