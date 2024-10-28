#version 460
#include "bindless.glsl"
#include "lighting.glsl"
#include "permutations.glsl"

layout (location = 0) in vec3 iPosition_ws;
#ifdef USE_TEXTURE_MAPS
	layout (location = 1) in vec2 iTexCoord;
#endif 
#ifdef USE_NORMAL_MAP
	layout (location = 2) in mat3 iTBN;
#else
	layout (location = 2) in vec3 iNormal;
#endif

layout (location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	layout(offset = 8)
	uint directionalLightBufferIdx;
	uint pointLightBufferIdx;
	uint spotLightBufferIdx;

	uint directionalLightCount;
	uint pointLightCount;
	uint spotLightCount;

	vec4 cameraViewPos;

#ifdef USE_DIFFUSE_MAP
	uint albedoTexIdx;
#else
	uint color; // RGBA packed 4x8
#endif

#ifdef USE_METALLIC_MAP
	uint metallicTexIdx;
#else 
	float metallic;
#endif

#ifdef USE_ROUGHNESS_MAP
	uint roughnessTexIdx;
#else 
	float roughness;
#endif

#ifdef USE_NORMAL_MAP
	uint normalTexIdx;
#else
	// uint normalDummy;
#endif 

#ifdef USE_AO_MAP
	uint aoTexIdx;
#else
	float ao;
#endif

#ifdef USE_EMISSIVE_MAP
	uint emissiveTexIdx;
#else
	vec4 emissive; // a unused
#endif

	// uint shadowTexCount;
};

RegisterBuffer(std430, readonly, DirectionalLightBuffer, { DirectionalLight directionalLightData[]; });
RegisterBuffer(std430, readonly, PointLightBuffer, { PointLight pointLightData[]; });
RegisterBuffer(std430, readonly, SpotLightBuffer, { SpotLight spotLightData[]; });

void main()	
{
	#ifdef USE_DIFFUSE_MAP
		vec4 texelColor = texture(uTextures2D[albedoTexIdx], iTexCoord);
	#else
		vec4 texelColor = unpackUnorm4x8(color);
	#endif
	#ifdef USE_METALLIC_MAP
		float metallicValue = texture(uTextures2D[metallicTexIdx], iTexCoord).r;
	#else
		float metallicValue = metallic;
	#endif
	#ifdef USE_ROUGHNESS_MAP
		float roughnessValue = texture(uTextures2D[roughnessTexIdx], iTexCoord).r;
	#else
		float roughnessValue = roughness;
	#endif
	#ifdef USE_NORMAL_MAP
		vec3 normalColor = texture(uTextures2D[normalTexIdx], iTexCoord).rgb;
	#else
		vec3 normalColor = vec3(0.0f, 0.0f, 1.0f);
	#endif

	#ifdef USE_AO_MAP
		float aoValue = texture(uTextures2D[aoTexIdx], iTexCoord).r;
	#else
		float aoValue = ao;
	#endif

	#ifdef USE_EMISSIVE_MAP
		vec3 emissiveColor = texture(uTextures2D[emissiveTexIdx], iTexCoord).rgb;
	#else
		vec3 emissiveColor = emissive.rgb;
	#endif

	/* ---------------- Lighting ---------------- */
	vec3 lightIntensity = vec3(0.0f);
	#ifdef USE_NORMAL_MAP
		vec3 normal_ts = normalize(iTBN * (normalColor * 2.0f - 1.0f));
	#else
		vec3 normal_ts = iNormal;
	#endif
	vec3 viewDir_ws = normalize(cameraViewPos.xyz - iPosition_ws);

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
	oColor = vec4(lightIntensity * texelColor.rgb + emissiveColor, texelColor.a);
}