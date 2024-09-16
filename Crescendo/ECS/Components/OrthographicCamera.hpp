#pragma once
#include "common.hpp"
#include "Component.hpp"
#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	struct OrthographicCamera : public Component
	{
		float left, right, bottom, top, nearPlane, farPlane;

		OrthographicCamera() : left(-1.0f), right(1.0f), bottom(-1.0f), top(1.0f), nearPlane(0.1f), farPlane(1000.0f) {}
		OrthographicCamera(float left, float right, float bottom, float top, float nearPlane, float farPlane) : left(left), right(right), bottom(bottom), top(top), nearPlane(nearPlane), farPlane(farPlane) {}

		cs_std::math::mat4 GetProjectionMatrix() const { return cs_std::math::ortho(left, right, bottom, top, nearPlane, farPlane); }
		cs_std::math::mat4 GetViewProjectionMatrix(const cs_std::math::mat4& viewMatrix) const { return GetProjectionMatrix() * viewMatrix; }
	};
}