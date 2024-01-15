#pragma once

#include "common.hpp"
#include "Component.hpp"
#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	struct PerspectiveCamera : public Component
	{
		float fov, aspectRatio, nearPlane, farPlane;
		PerspectiveCamera() : fov(45.0f), aspectRatio(16.0f / 9.0f), nearPlane(0.1f), farPlane(1000.0f) {}
		PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane) : fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {}

		// We use reverse-Z
		cs_std::math::mat4 GetProjectionMatrix() const
		{
			constexpr cs_std::math::mat4 reverseZ{ 1.0f, 0.0f,  0.0f, 0.0f,
							0.0f, 1.0f,  0.0f, 0.0f,
							0.0f, 0.0f, -1.0f, 0.0f,
							0.0f, 0.0f,  1.0f, 1.0f };
			return reverseZ * cs_std::math::perspective(fov, aspectRatio, nearPlane, farPlane);
		}
		cs_std::math::mat4 GetViewProjectionMatrix(const cs_std::math::mat4& viewMatrix) const { return GetProjectionMatrix() * viewMatrix; }
	};
}