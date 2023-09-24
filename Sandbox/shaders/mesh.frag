#version 450
layout (location = 0) in vec2 iTexCoord;
// layout (location = 1) out vec3 oNormal;
layout (location = 1) in vec3 iTanFragPos;
layout (location = 2) in vec3 iTanLightPos;
layout (location = 3) in vec3 iTanViewPos;
layout (location = 4) in vec4 iFragPosLightSpace;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform blinnPhongLightingData {
	vec4 lightColor;
};

layout(set = 1, binding = 1) uniform blinnPhongLightingIntensities {
	float ambient;
	float diffuse;
	float specular;
};

layout(set = 2, binding = 0) uniform sampler2D albedoTex;
layout(set = 3, binding = 0) uniform sampler2D normalTex;
// layout(set = 4, binding = 0) uniform sampler2D shadowTex;

void main()
{
	vec4 texelColor = texture(albedoTex, iTexCoord);

	/* ---------------- Normals ---------------- */

	vec3 normal = texture(normalTex, iTexCoord).rgb;
	normal = normalize(normal * 2.0f - 1.0f);

	/* ---------------- Lighting ---------------- */

	// Ambient
	vec3 ambient = ambient * lightColor.rgb;

	// Diffuse
	vec3 lightDir = normalize(iTanLightPos - iTanFragPos);
	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diffuse * diff * lightColor.rgb;

	// Specular
	vec3 viewDir = normalize(iTanViewPos - iTanFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f);
	vec3 specular = specular * spec * lightColor.rgb;

	// Lighting mixing
	vec3 result = (ambient + diffuse + specular) * texelColor.rgb;

	/* ---------------- Shadows ---------------- */
	/*
	vec3 projCoords = (iFragPosLightSpace.xyz / iFragPosLightSpace.w) * 0.5f + 0.5f; // map to [0, 1] range
	float closestDepth = texture(shadowTex, projCoords.xy).r;
	// float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = projCoords.z > closestDepth ? 1.0 : 0.2;
	result *= shadow;
	*/
	/* ---------------- Output ---------------- */

	// Output
	oColor = vec4(result, texelColor.a);
}