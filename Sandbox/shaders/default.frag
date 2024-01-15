#version 460

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

layout(set = 2, binding = 0) uniform Camera { vec4 viewPos; }; // a unused
layout(set = 3, binding = 0) uniform LightCounts { uint directionalLightCount, pointLightCount, spotLightCount; };

layout(std140, set = 4, binding = 0) readonly buffer DirectionalLightStorage { DirectionalLight directionalLights[]; };
layout(std140, set = 5, binding = 0) readonly buffer PointLightStorage { PointLight pointLights[]; };
layout(std140, set = 6, binding = 0) readonly buffer SpotLightStorage { SpotLight spotLights[]; };

layout(set = 7, binding = 0) uniform sampler2D diffuseTex;
layout(set = 8, binding = 0) uniform sampler2D normalTex;
// layout(set = 9, binding = 0) uniform sampler2D shadowTexs[];

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
	return intensity / (distanceSquared + 1);
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal_ws, vec3 viewDir_ws)
{
	const float intensity = light.direction.w;

	vec3 lightDir_ws = light.direction.xyz;
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);

	float diff = max(dot(normal_ws, lightDir_ws), 0.0f);
	float spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 128.0f);

	vec3 diffuse = vec3(diff);
	vec3 specular = vec3(spec);
	return (diffuse + specular) * light.color.rgb * intensity;
}

vec3 CalcPointLight(PointLight light, vec3 normal_ws, vec3 viewDir_ws, vec3 fragmentPosition_ws)
{
	const float intensity = light.position.w;

	vec3 lightDir_ws = normalize(iTBN * light.position.xyz - iTBN * fragmentPosition_ws);
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);
	float attenuation = lightAttenuation(intensity, distSqrd(light.position.xyz, fragmentPosition_ws));

	float diff = max(dot(normal_ws, lightDir_ws), 0.0f);
	float spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 128.0f);

	return (diff + spec) * light.color.rgb * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal_ws, vec3 viewDir_ws, vec3 fragmentPosition_ws)
{
	const float spotAngle_cos = light.direction.w;
	const float fadeAngle_cos = light.color.a;
	const float intensity = light.position.w;

	vec3 lightDir_ws = normalize(iTBN * light.position.xyz - iTBN * fragmentPosition_ws);
	vec3 reflectDir_ws = reflect(-lightDir_ws, normal_ws);
	float attenuation = lightAttenuation(intensity, distSqrd(light.position.xyz, fragmentPosition_ws));

	float theta = dot(normalize(light.position.xyz - fragmentPosition_ws), normalize(-light.direction.rgb)); 
    float epsilon = spotAngle_cos - fadeAngle_cos;
    float intensityFactor = clamp((theta - fadeAngle_cos) / epsilon, 0.0, 1.0);

	float diff = max(dot(normal_ws, lightDir_ws), 0.0f);
	float spec = pow(max(dot(viewDir_ws, reflectDir_ws), 0.0f), 128.0f);

	return (diff + spec) * light.color.rgb * attenuation * intensityFactor;
}

void main()	
{
	vec4 texelColor = texture(diffuseTex, iTexCoord);
	/* ---------------- Lighting ---------------- */

	vec3 normal_ws = normalize((texture(normalTex, iTexCoord).rgb) * 2.0f - 1.0f);
	vec3 viewDir_ws = normalize(viewPos.xyz - iPosition_ws);

	vec3 outputColor = vec3(0.0f);

	// Directional lights
	for (uint i = 0; i < directionalLightCount; i++)
	{
		outputColor += CalcDirectionalLight(directionalLights[i], normal_ws, viewDir_ws);
	}

	// Point lights
	for (uint i = 0; i < pointLightCount; i++)
	{
		outputColor += CalcPointLight(pointLights[i], normal_ws, viewDir_ws, iPosition_ws);
	}

	// Spot lights
	for (uint i = 0; i < spotLightCount; i++)
	{
		outputColor += CalcSpotLight(spotLights[i], normal_ws, viewDir_ws, iPosition_ws);
	}

	/* ---------------- Shadows ---------------- */
	
	// float shadow = textureProj(iFragPosLightSpace / iFragPosLightSpace.w);
	// result *= shadow;
	
	/* ---------------- Output ---------------- */

	// Output
	oColor = vec4(outputColor * texelColor.rgb, texelColor.a);
}