*Spoopy*
![[Lights pt.1 - 1.png]]

The [LearnOpenGL](https://learnopengl.com) website has a great [article](https://learnopengl.com/Lighting/Light-casters)describing the main 4 lights typically used in rendering. Directional, Point and Spot Lights. Each of them have their own use cases.
### Directional Light
Usually used for sunlight. These are simulated using parallel light rays using an infinitely distant light. Very easy to implement and shadow map
### Point Light
Used for general lights such as a lightbulb or candle
### Spot light
Used for flashlights or street lights
### Area Light
Rectangular light, More expensive but looks very nice. While I will not be implementing area lights in this article, a follow-up article will go in depth, likely after PBR.
## Attenuation
In the real world, lights have area and are thus subject to the inverse square law:
$$
I = \frac{P}{4\pi r^2}
$$
Where: I is intensity, P is power, r is radius. 

The inverse square law shows us that as the distance to a light source doubles, the intensity falls off four-fold. However in game engines, lights such as point and spot lights have 0 physical area. Hence, if applying the inverse square law, as the distances approach 0, the intensity approaches infinity. Not good! We need a custom solution for this

Different engines and different renderers handle lights differently. This [article](https://geom.io/bakery/wiki/index.php?title=Point_Light_Attenuation) shows us what different engines use:
### Unity URP:
The Unity Universal Render Pipeline (URP) uses a fake falloff curve that allows specifying a light's maximum range:
$$
\frac{1}{\left(\left(\frac{x}{r}\right) \cdot 0.5\right)^2 + 1}
$$
I like this equation a bit as it allows for artist control on the maximum distance of lights. However there is some contention on the forums as to the realism of the light algorithm. Hence we will not use this algorithm.

## Unity "Bakery" & Unreal Engine 4:
Unity's "bakery" light baker and UE4 use a formula similar to the physical falloff but with an added +1 to prevent divide by 0 issues:
$$
\frac{1}{(x \cdot x + 1)}
$$
However the article mentions that the +1 constant is arbitrary and could look different based on scene size/units. However this is fine and is the algorithm I use in Crescendo.
## Unity HDRP & Frostbite
One other algorithm I saw and considered but ultimately did not use has a customizable parameter for the light size, which defaults to 0.01 (1cm):
$$
\frac{1}{\max(x, s)^2}
$$
This means that the brightest possible light has a intensity value of 10,000. I decided against this as it's an extra shader calculation and requires repacking parameters.

## [LearnOpenGL](https://learnopengl.com), Blender & Ogre
[LearnOpenGL](https://learnopengl.com), Blender & Ogre uses and allows for a more finely controlled attenuation using 3 parameters: a constant, linear and quadratic falloff
$$
qx^2 + lx + c
$$
Where q is a quadratic coefficient, l is a linear coefficient and c is a constant. While this isn't what we are looking for as it is non-physically based and has more of parameters to tune, this equation is quite useful for custom rendering solutions or just simple lights.
## Implementation
In my current implementation. We limit each lights to a maximum, specified by the config.xml file:
- Directional light: 16 lights.
- Point light: 256 lights.
- Spot light: 256 lights.
These limits are arbitrarily set. Mainly because we have no SSBO resizing capabilities yet in my engine. On my Arc A730M, it can handle a maximum of 64 total lights of any type (Without shadow mapping) at 1600p@144fps. Which is honestly rather impressive for a forward renderer + depth prepass.
![[Lights pt.1 - 2.png]]

In Crescendo I upgraded the renderer to be able to make arbitrary SSBOs
```C++
struct SSBO
{
	Buffer buffer;
	VkDescriptorSet set;
	size_t size;
};
```
```C++
class VulkanInstance
{
	...
public:
	// Returns the index to the ssbo
	uint32_t CreateSSBO(size_t size, VkShaderStageFlags shaderStage);
	...
};
```
Hence here we create the object buffer and the SSBOs
```C++
for (uint32_t i = 0; i < this->renderer.specs.framesInFlight; i++)
{
	this->transformSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(Transform) * CVar::Get<uint64_t>("irc_maxobjectcount"), VK_SHADER_STAGE_VERTEX_BIT));
	this->directionalLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(DirectionalLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxdirectionallightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
	this->pointLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(PointLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxpointlightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
	this->spotLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(SpotLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxspotlightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
}
```

Since our engine is ECS now we defined components for each of the lights. as well as their packed representations. Each of these also require a transform component attached to the entity to be able to create their shader representations.

```C++
struct DirectionalLight : public Component
{
	struct ShaderRepresentation
	{
		cs_std::math::vec4 direction; // x, y, z, intensity
		cs_std::math::vec4 color; // a unused
	};
	cs_std::math::vec3 color;
	float intensity;
	bool isShadowCasting;
	...
};
```
```C++
struct PointLight : public Component
{
	struct ShaderRepresentation
	{
		cs_std::math::vec4 position; // x, y, z, intensity
		cs_std::math::vec4 color; // a unused
	};

	cs_std::math::vec3 color;
	float intensity;
	bool isShadowCasting;
	...
};
```
```C++
struct SpotLight : public Component
{
	struct ShaderRepresentation
	{
		cs_std::math::vec4 position; // x, y, z, intensity
		cs_std::math::vec4 direction; // x, y, z, spotAngle
		cs_std::math::vec4 color; // r, g, b, fadeAngle
	};

	cs_std::math::vec3 color;
	float intensity;
	float spotAngle;
	float fadeAngle;
	bool isShadowCasting;
	...
};
```

Very nice. Now in the next stage, render preparation. We use a wrapper around Entt's views to iterate and find the counts of each light. and we fill the SSBOs naively by just copying the data directly. This is ok for now as it's not an expensive operation but we may want to change this if we find performance caveats or move GPU only memory

```C++
std::vector<DirectionalLight::ShaderRepresentation> directionalLights;
std::vector<PointLight::ShaderRepresentation> pointLights;
std::vector<SpotLight::ShaderRepresentation> spotLights;

entityManager.ForEach<Transform, DirectionalLight>([&](entt::entity e, Transform& transform, DirectionalLight& light) {
	// Data packing
	if (directionalLights.size() >= this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(DirectionalLight::ShaderRepresentation)) return;
	directionalLights.push_back(light.CreateShaderRepresentation(transform));
});

entityManager.ForEach<Transform, PointLight>([&](entt::entity e, Transform& transform, PointLight& light) {
	// Data packing
	if (pointLights.size() >= this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(PointLight::ShaderRepresentation)) return;
	pointLights.push_back(light.CreateShaderRepresentation(transform));
});

entityManager.ForEach<Transform, SpotLight>([&](entt::entity e, Transform& transform, SpotLight& light) {
	// Data packing
	if (spotLights.size() >= this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(SpotLight::ShaderRepresentation)) return;
	spotLights.push_back(light.CreateShaderRepresentation(transform));
});

this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());
```
### Shader
At this point, all we need to do is bind it to the shaders, of which, I need to explain the shader part

```C
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

void main()	
{
	vec4 texelColor = texture(diffuseTex, iTexCoord);
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
	// Output
	oColor = vec4(outputColor * texelColor.rgb, texelColor.a);
}
```

Here in the shader, we just loop over each of the lights and add their light values together, before we finally output the color multiplied by the texel color. While I have not shown it here, the frame buffer being rendered to is a HDR format, there is a postprocessing tone mapper being applied to prevent the colors from clipping and showing pure white in bright lights.

One thing, you may have also noticed is the 2 letter names appended to some variables after an underscore. This is used to record the coordinate space the variable is in. This caused many infuriating issues with point lights not rendering correctly but it helps immensely. For further reading, read [[Know your space]].
### Helper functions 
```C
float distSqrd(vec3 a, vec3 b)
{
	vec3 dist = a - b;
	return dot(dist, dist);
}

float lightAttenuation(float intensity, float distanceSquared)
{
	return intensity / (distanceSquared + 1);
}
```
These helper functions just reduce code complexity and improve performance (imagine computing a square root, only to square it again)

### Light calculations
One difference you may notice is that I did not calculate for ambient lights. While I may consider it in the future, for now everything without a light is in darkness
#### Directional light calculation
```C
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
```
#### Point light calculation
```C++
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
```
#### Spot light calculation
The calculations for spot lights are largely the same as point lights. Just an extra statement to determine if the fragment is inside the spot light cone
```C
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
```
![[Lights pt.1 - 3.png]]