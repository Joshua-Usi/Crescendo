#pragma once

#include "common.hpp"
#include "Component.hpp"

#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	// Position and direction is derived from the transform
	// Spot angle is in radians, spot angle is the angle from edge to edge of the cone
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

		SpotLight() : color(1.0f, 1.0f, 1.0f), intensity(1.0f), spotAngle(cs_std::math::half_pi<float>()), fadeAngle(cs_std::math::half_pi<float>()), isShadowCasting(true) {}
		SpotLight(const cs_std::math::vec3& color, float intensity, float spotAngle, float fadeAngle, bool isShadowCasting) : color(color), intensity(intensity), spotAngle(spotAngle), fadeAngle(fadeAngle), isShadowCasting(isShadowCasting) {}

		ShaderRepresentation CreateShaderRepresentation(const Transform& transform) const
		{
			return
			{
				cs_std::math::vec4(transform.GetPosition(), intensity),
				cs_std::math::vec4(transform.GetDirection(), cs_std::math::cos(spotAngle)),
				cs_std::math::vec4(color, cs_std::math::cos(fadeAngle))
			};
		}
	};
}