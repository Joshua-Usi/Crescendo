#version 460
#include "bindless.glsl"

struct DirectionalLight {
	vec4 direction; // x, y, z, intensity
	vec4 color; // a unused
};

struct PointLight {
	vec4 position; // x, y, z, intensity
	vec4 color; // a unused
};

struct SpotLight {
	vec4 position; // x, y, z, intensity
	vec4 direction; // x, y, z, cos(spotAngle), end to end
	vec4 color; // r, g, b, cos(fadeAngle)
};

layout (location = 0) in vec3 iPosition_ws;
layout (location = 1) in vec2 iTexCoord;
layout (location = 2) in mat3 iTBN;

layout (location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	layout(offset = 8) uint diffuseTexIdx;
	uint normalTexIdx;

	uint directionalLightBufferIdx;
	uint pointLightBufferIdx;
	uint spotLightBufferIdx;

	uint directionalLightCount;
	uint pointLightCount;
	uint spotLightCount;

	uint dummy0;
	uint dummy1;

	vec4 cameraViewPos;
	// uint shadowTexCount;
};

RegisterBuffer(std430, readonly, DirectionalLightBuffer, { DirectionalLight directionalLightData[]; });
RegisterBuffer(std430, readonly, PointLightBuffer, { PointLight pointLightData[]; });
RegisterBuffer(std430, readonly, SpotLightBuffer, { SpotLight spotLightData[]; });

// Standard hard shadows
// float textureProj(sampler2D shadowTex, vec4 shadowCoord)
// {
//     float shadow = 1.0f;
//     float dist = texture(shadowTex, shadowCoord.xy).r;
//     float condition = step(0.0, shadowCoord.w) * step(shadowCoord.z - 0.002f, dist);
//     shadow = mix(0.4f, shadow, condition);
//     return shadow;
// }

float distSqrd(vec3 a, vec3 b)
{
	vec3 dist = a - b;
	return dot(dist, dist);
}

float lightAttenuation(float intensity, float distanceSquared)
{
	return intensity / (distanceSquared + 1.0);
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal_ws, vec3 viewDir_ws)
{
	const float intensity = light.direction.w;

	vec3 lightDir_ws = light.direction.xyz;
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);

	vec3 diff = max(dot(normal_ws, lightDir_ws), 0.0f) * light.color.rgb;
	vec3 spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 32.0f) * light.color.rgb;

	return (diff + spec) * intensity;
}

vec3 CalcPointLight(PointLight light, vec3 normal_ws, vec3 viewDir_ws, vec3 fragmentPosition_ws)
{
	const float intensity = light.position.w;

	vec3 lightDir_ws = normalize(light.position.xyz - fragmentPosition_ws);
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);
	float attenuation = lightAttenuation(intensity, distSqrd(light.position.xyz, fragmentPosition_ws));

	vec3 diff = max(dot(normal_ws, lightDir_ws), 0.0f) * light.color.rgb;
	vec3 spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 32.0f) * light.color.rgb;

	return (diff + spec) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal_ws, vec3 viewDir_ws, vec3 fragmentPosition_ws)
{
	const float spotAngle_cos = light.direction.w;
	const float fadeAngle_cos = light.color.a;
	const float intensity = light.position.w;

	vec3 lightDir_ws = normalize(light.position.xyz - fragmentPosition_ws);
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);
	float attenuation = lightAttenuation(intensity, distSqrd(light.position.xyz, fragmentPosition_ws));

	float theta = dot(lightDir_ws, normalize(-light.direction.rgb)); 
    float epsilon = spotAngle_cos - fadeAngle_cos;
    float intensityFactor = clamp((theta - fadeAngle_cos) / epsilon, 0.0, 1.0);

	vec3 diff = max(dot(normal_ws, lightDir_ws), 0.0f) * light.color.rgb;
	vec3 spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 32.0f) * light.color.rgb;

	return (diff + spec) * attenuation * intensityFactor;
}

void main()	
{
	vec4 texelColor = texture(uTextures2D[diffuseTexIdx], iTexCoord);
	vec4 normalColor = texture(uTextures2D[normalTexIdx], iTexCoord);
	/* ---------------- Lighting ---------------- */

	vec3 normal_ts = normalize(iTBN * (normalColor.rgb * 2.0f - 1.0f));
	vec3 viewDir_ws = normalize(cameraViewPos.xyz - iPosition_ws);

	vec3 lightIntensity = vec3(0.2f);

	// Directional lights
	for (uint i = 0; i < directionalLightCount; i++)
	{
		const DirectionalLight directionalLight = GetResource(DirectionalLightBuffer, directionalLightBufferIdx).directionalLightData[i];
		lightIntensity += CalcDirectionalLight(directionalLight, normal_ts, viewDir_ws);
	}

	// Point lights
	for (uint i = 0; i < pointLightCount; i++)
	{
		const PointLight pointLight = GetResource(PointLightBuffer, pointLightBufferIdx).pointLightData[i];
		lightIntensity += CalcPointLight(pointLight, normal_ts, viewDir_ws, iPosition_ws);
	}

	// Spot lights
	for (uint i = 0; i < spotLightCount; i++)
	{
		const SpotLight spotlight = GetResource(SpotLightBuffer, spotLightBufferIdx).spotLightData[i];
		lightIntensity += CalcSpotLight(spotlight, normal_ts, viewDir_ws, iPosition_ws);
	}

	/* ---------------- Shadows ---------------- */
	
	// float shadow = textureProj(iFragPosLightSpace / iFragPosLightSpace.w);
	// result *= shadow;
	
	/* ---------------- Output ---------------- */

	// Output
	oColor = vec4(lightIntensity * texelColor.rgb, texelColor.a);
}