/*
 *	Contains functions relevant to light intensity calculations
 */

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