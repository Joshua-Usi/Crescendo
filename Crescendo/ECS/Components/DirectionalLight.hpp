#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "Transform.hpp"
#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	// Direction is derived from the transform
	// If the light is not shadow mapped it will light objects behind other objects
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

		DirectionalLight() : color(1.0f, 1.0f, 1.0f), intensity(1.0f), isShadowCasting(true) {}
		DirectionalLight(const cs_std::math::vec3& color, float intensity, bool isShadowCasting) : color(color), intensity(intensity), isShadowCasting(isShadowCasting) {}

		ShaderRepresentation CreateShaderRepresentation(const Transform& transform) const
		{
			return
			{
				cs_std::math::vec4(transform.GetDirection(), intensity),
				cs_std::math::vec4(color, 0.0f)
			};
		}
	};
}