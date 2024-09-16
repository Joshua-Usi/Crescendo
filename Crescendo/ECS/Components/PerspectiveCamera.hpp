#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	struct alignas(16) PerspectiveCamera : public Component
	{
		float fov, nearPlane, farPlane;
		PerspectiveCamera() : fov(45.0f), nearPlane(0.1f), farPlane(1000.0f) {}
		PerspectiveCamera(float fov, float nearPlane, float farPlane) : fov(fov), nearPlane(nearPlane), farPlane(farPlane) {}

		// We use reverse-Z
		cs_std::math::mat4 GetProjectionMatrix(float aspectRatio) const
		{
			constexpr cs_std::math::mat4 reverseZ {
				1.0f, 0.0f,  0.0f, 0.0f,
				0.0f, 1.0f,  0.0f, 0.0f,
				0.0f, 0.0f, -1.0f, 0.0f,
				0.0f, 0.0f,  1.0f, 1.0f
			};
			return reverseZ * cs_std::math::perspective(fov, aspectRatio, nearPlane, farPlane);
		}
		cs_std::math::mat4 GetViewProjectionMatrix(float aspectRatio, const cs_std::math::mat4& viewMatrix) const { return GetProjectionMatrix(aspectRatio) * viewMatrix; }
	};
}