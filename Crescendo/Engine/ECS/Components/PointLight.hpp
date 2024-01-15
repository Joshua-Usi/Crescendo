#pragma once

#include "common.hpp"
#include "Component.hpp"

#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	// Position is derived from the transform
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

		PointLight() : color(1.0f, 1.0f, 1.0f), intensity(1.0f), isShadowCasting(true) {}
		PointLight(const cs_std::math::vec3& color, float intensity, bool isShadowCasting) : color(color), intensity(intensity), isShadowCasting(isShadowCasting) {}

		ShaderRepresentation CreateShaderRepresentation(const Transform& transform) const
		{
			return
			{
				cs_std::math::vec4(transform.GetPosition(), intensity),
				cs_std::math::vec4(color, 0.0f)
			};
		}
	};
}