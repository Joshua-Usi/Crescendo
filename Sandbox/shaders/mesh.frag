#version 450
layout (location = 0) in vec2 iTexCoord;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec3 iTanFragPos;
layout (location = 3) in vec3 iTanLightPos;
layout (location = 4) in vec3 iTanViewPos;
layout (location = 5) in vec4 iFragPosLightSpace;

layout (location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform blinnPhongLightingIntensities {
	float ambient;
	float diffuse;
	float specular;
} bpli;

layout(set = 2, binding = 0) uniform sampler2D diffuseTex;
layout(set = 3, binding = 0) uniform sampler2D normalTex;
layout(set = 4, binding = 0) uniform sampler2D shadowTex;

float textureProj(vec4 shadowCoord)
{
    float shadow = 1.0;
    float dist = texture(shadowTex, shadowCoord.xy).r;

    float condition = step(0.0, shadowCoord.w) * step(shadowCoord.z - 0.002f, dist);
    shadow = mix(bpli.ambient, shadow, condition);

    return shadow;
}


void main()
{
	vec4 texelColor = texture(diffuseTex, iTexCoord);

	/* ---------------- Normals ---------------- */

	vec3 normal = texture(normalTex, iTexCoord).rgb;
	normal = normalize(normal * 2.0f - 1.0f);

	/* ---------------- Lighting ---------------- */

	// Diffuse
	vec3 lightDir = normalize(iTanLightPos - iTanFragPos);
	float diff = max(dot(normal, lightDir), 0.0f);
	float diffuse = bpli.diffuse * diff;

	// Specular
	vec3 viewDir = normalize(iTanViewPos - iTanFragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32.0f);
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