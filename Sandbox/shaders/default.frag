#version 460
layout (location = 0) in vec2 iTexCoord;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec3 iTanFragPos;
layout (location = 3) in vec3 iTanLightPos;
layout (location = 4) in vec3 iTanViewPos;
layout (location = 5) in vec4 iFragPosLightSpace;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform LightIntensities {
	float ambient;
	float diffuse;
	float specular;
} bpli;

layout(set = 3, binding = 0) uniform sampler2D diffuseTex;
layout(set = 4, binding = 0) uniform sampler2D normalTex;
layout(set = 5, binding = 0) uniform sampler2D shadowTex;

// Standard hard shadows
float textureProj(vec4 shadowCoord)
{
    float shadow = 10.0;
    float dist = texture(shadowTex, shadowCoord.xy).r;

    float condition = step(0.0, shadowCoord.w) * step(shadowCoord.z - 0.002f, dist);
    shadow = mix(bpli.ambient, shadow, condition);

    return shadow;
}

// PCF soft shadows
float textureProjPCF(vec4 shadowCoord)
{
	const int PCF_SIZE = 5;
	vec2 filterSize = 1.0f / textureSize(shadowTex, 0);
	float shadow = 0.0;

    for (int i = -PCF_SIZE; i <= PCF_SIZE; i++)
	{
        for (int j = -PCF_SIZE; j <= PCF_SIZE; j++)
		{
            shadow += step(shadowCoord.z - 0.002f, texture(shadowTex, vec2(shadowCoord.x + i * filterSize.x, shadowCoord.y + j * filterSize.y)).r);
        }
    }
    shadow /= float((PCF_SIZE * 2 + 1) * (PCF_SIZE * 2 + 1));
    float inShadowMap = step(0.0f, shadowCoord.w);
    float lightIntensity = mix(bpli.ambient, 10.0f, shadow);
    lightIntensity = mix(lightIntensity, 1.0f, 1.0f - inShadowMap);

    return lightIntensity;
}

// Vogel disk soft shadows
vec2 vogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
	const float goldenAngle = 2.4f;
	float r = sqrt(float(sampleIndex) / float(samplesCount));
	float theta = float(sampleIndex) * goldenAngle + phi;

	return vec2(r * cos(theta), r * sin(theta));
}
float textureProjVogel(vec4 shadowCoord)
{
	const int SAMPLES_COUNT = 32;
	vec2 filterSize = 1.0f / textureSize(shadowTex, 0);
	float shadow = 0.0;

	for (int i = 0; i < SAMPLES_COUNT; ++i)
	{
        vec2 sampleOffset = vogelDiskSample(i, SAMPLES_COUNT, 0.0) * filterSize;
        vec2 samplePos = shadowCoord.xy + sampleOffset;
        shadow += step(shadowCoord.z - 0.002f, texture(shadowTex, samplePos).r);
    }

	shadow /= float(SAMPLES_COUNT);
	float inShadowMap = step(0.0f, shadowCoord.w);
	float lightIntensity = mix(bpli.ambient, 10.0f, shadow);
	lightIntensity = mix(lightIntensity, 1.0f, 1.0f - inShadowMap);

	return lightIntensity;

}


void main()
{
	vec4 texelColor = texture(diffuseTex, iTexCoord);

	/* ---------------- Normal mapped ---------------- */

	vec3 normal = texture(normalTex, iTexCoord).rgb;
	normal = normalize(normal * 2.0f - 1.0f);

	/* ---------------- Lighting - Blinn-Phong ---------------- */

	// Diffuse
	vec3 lightDir = normalize(iTanLightPos - iTanFragPos);
	float diff = max(dot(normal, lightDir), 0.0f);
	float diffuse = bpli.diffuse * diff;

	// Specular
	vec3 viewDir = normalize(iTanViewPos - iTanFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
	float specular = bpli.specular * spec;

	// Lighting mixing
	vec3 result = (bpli.ambient + diffuse + specular) * texelColor.rgb;

	/* ---------------- Shadows ---------------- */
	
	float shadow = textureProj(iFragPosLightSpace / iFragPosLightSpace.w);
	result *= shadow;
	
	/* ---------------- Output ---------------- */

	// Output
	oColor = vec4(result, texelColor.a);
}