*Floats are weird*

While my engine is still in pre-alpha days, I still dream of a time where I can render metaverse-scale open worlds in my engine. Alas for now, I have to start small and am stuck with this small sponza scene.

![[Reverse-Z - Sponza Scene.jpg]]

However even at this small scale. Z-fighting issues can still occur and that's with pitiful render distances. With a near and far plane of `0.1f` and `1000.0f` respectively. Z-fighting still occurs.

This isn't any fault to do with us. But rather with the way precision is distributed in floating point numbers.

This [article](https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html) goes into further detail with the mathematics and the precisions but as a reference in the range `0.0` to `1.0`, The range of numbers representable from `0.5` to `1.0` is a measly `0.79%` while on the other hand the range `0.0` to `0.5` is a "staggering **`99.21%`**".

The main issue comes from classical projection representing near objects depth with `0.0` and far objects with `1.0`. Far objects have a 100x less precision than near objects.

Hence the technique reverse-Z was born. 

In all graphics APIs, 3 high-level things need to be done to correctly implement Reverse-Z (4 in OpenGL but we won't go into that since we use Vulkan):
1. 1. Clear depth to 0 (not 1 as usual).
2. Set depth test to _greater_ (not _less_ as usual).
3. Ensure you’re using a floating point depth buffer.

Here is how it was implemented in the engine:

1. For perspective cameras, it is as simple as flipping around the arguments (cs_std::math is equivalent to glm) (*Engine/ECS/Components.hpp*):
```Diff
struct PerspectiveCamera : public Component
{
...
	cs_std::math::mat4 GetProjectionMatrix() const
	{
+		constexpr cs_std::math::mat4 reverseZ{ 1.0f, 0.0f,  0.0f, 0.0f,
+				0.0f, 1.0f,  0.0f, 0.0f,
+				0.0f, 0.0f, -1.0f, 0.0f,
+				0.0f, 0.0f,  1.0f, 1.0f };
+		return reverseZ * cs_std::math::perspective(fov, aspectRatio, nearPlane, farPlane);
-		return cs_std::math::perspective(fov, aspectRatio, nearPlane, farPlane);
	}
...
};
```

2. Update the pipeline variants to use greater rather than less comparison, cbf to create diff (*Rendering/Vulkan/Types/PipelineVariants.hpp*):
```C++
static PipelineVariants GetDefaultVariant(VkRenderPass pass, uint32_t samples = 1)
{
	return PipelineVariants(pass, FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
}
static PipelineVariants GetSkyboxVariant(VkRenderPass pass, uint32_t samples = 1)
{
	return PipelineVariants(pass, FillMode::Solid, CullMode::Back, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
}
...
static PipelineVariants GetDepthPrepassVariant(VkRenderPass pass, uint32_t samples = 1)
{
	return PipelineVariants(pass, FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Enabled);
}
```

3.  Make the default depth clear `0.0f` (*Rendering/Vulkan/Types/Create.hpp*):
```diff
inline constexpr VkClearValue DefaultDepthClear()
{
	VkClearValue depthClear = {};
+	depthClear.depthStencil.depth = 0.0f;
-	depthClear.depthStencil.depth = 1.0f;
	return depthClear;
}
```

4.  Notify pipelines of the Z-reversal (*Rendering/Vulkan/Device.cpp*):
```diff
const PipelineBuilderInfo pipelineBuilderInfo = {
...
	.depthStencilInfo = Create::PipelineDepthStencilStateCreateInfo(
		0, PipelineVariants::GetDepthTest(thisVariant.depthTestFlags), PipelineVariants::GetDepthWrite(thisVariant.depthWriteFlags),
		PipelineVariants::GetDepthFunc(thisVariant.depthFuncFlags),
		VK_FALSE, VK_FALSE, {}, {},
+		1.0f, 0.0f // For reverse Z
	),
...
};
```

![[Reverse-Z - Sponza Scene.jpg]]

While there is no visible change yet, larger scenes will benefit greatly from this technique.