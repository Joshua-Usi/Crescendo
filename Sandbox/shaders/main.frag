#version 460
#include "bindless.glsl"
#include "lighting.glsl"

// Feature flags
// #define USE_DIFFUSE_MAP : use the diffuse texture, otherwise uses a solid color
// #define USE_METALLIC_MAP : use the metallic texture, otherwise uses a metallic value
// #define USE_ROUGHNESS_MAP : use the roughness map, otherwise use a roughness value
// #define USE_NORMAL_MAP : use the normal texture, otherwise the default normal. does not need to be defined if USE_LIGHTING is not
// #define USE_AO_MAP : use the ambient occlusion map, otherwise uses a ao value. usually 1.0f
// #define USE_EMISSIVE_MAP : uses the emissive texture, otherwise uses an emissive color

// #define USE_LIGHTING : object is subject to lighting calculations. otherwise it renders unlit with full ambient lighting

layout (location = 0) in vec3 iPosition_ws;
layout (location = 1) in vec2 iTexCoord;
layout (location = 2) in mat3 iTBN;

layout (location = 0) out vec4 oColor;

layout(push_constant) uniform PushConstants {
	layout(offset = 8)
#ifdef USE_LIGHTING
	uint directionalLightBufferIdx;
	uint pointLightBufferIdx;
	uint spotLightBufferIdx;

	uint directionalLightCount;
	uint pointLightCount;
	uint spotLightCount;

	vec4 cameraViewPos;
#endif

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

// Standard hard shadows
// float textureProj(sampler2D shadowTex, vec4 shadowCoord)
// {
//     float shadow = 1.0f;
//     float dist = texture(shadowTex, shadowCoord.xy).r;
//     float condition = step(0.0, shadowCoord.w) * step(shadowCoord.z - 0.002f, dist);
//     shadow = mix(0.4f, shadow, condition);
//     return shadow;
// }

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
		float aoValue = texture(uTextures2D[normalTexIdx], iTexCoord).r;
	#else
		float aoValue = ao;
	#endif

	#ifdef USE_EMISSIVE_MAP
		vec3 emissiveColor = texture(uTextures2D[emissiveTexIdx], iTexCoord).rgb;
	#else
		vec3 emissiveColor = emissive.rgb;
	#endif

	/* ---------------- Lighting ---------------- */
	#ifdef USE_LIGHTING
		vec3 lightIntensity = vec3(0.0f);
		vec3 normal_ts = normalize(iTBN * (normalColor * 2.0f - 1.0f));
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
	#else
		vec3 lightIntensity = vec3(1.0f);
	#endif

	/* ---------------- Shadows ---------------- */
	
	// float shadow = textureProj(iFragPosLightSpace / iFragPosLightSpace.w);
	// result *= shadow;
	
	/* ---------------- Output ---------------- */

	// Output
	oColor = vec4(lightIntensity * texelColor.rgb + emissiveColor, texelColor.a);
}