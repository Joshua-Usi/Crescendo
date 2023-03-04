#include "PerspectiveCamera.h"

#include "Mathematics/Rotations.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Crescendo::Rendering
{
	PerspectiveCamera::PerspectiveCamera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
		: projectionMatrix(glm::perspective(fieldOfView, aspectRatio, nearPlane, farPlane)),
		  viewMatrix(glm::mat4(1.0f)),
		  position(glm::vec3(0.0f, 0.0f, 0.0f)),
		  rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
	{}
	const glm::vec3& PerspectiveCamera::GetPosition() const
	{
		return this->position;
	}
	const glm::vec3 PerspectiveCamera::GetRotation() const
	{
		return Mathematics::QuaternionToEuler(this->rotation);
	}
	const glm::quat& PerspectiveCamera::GetQuaternionRotation() const
	{
		return this->rotation;
	}

	void PerspectiveCamera::SetPosition(const glm::vec3& newPosition)
	{
		this->position = newPosition;
		this->RecalculateMatrices();
	}
	void PerspectiveCamera::SetRotation(const glm::vec3& euler)
	{
		this->rotation = Mathematics::EulerToQuaternion(euler);
		this->RecalculateMatrices();
	}
	void PerspectiveCamera::SetRotation(const glm::quat& quaternion)
	{
		this->rotation = quaternion;
		this->RecalculateMatrices();
	}
	void PerspectiveCamera::RecalculateMatrices()
	{
		glm::mat4 orientation = glm::mat4_cast(glm::inverse(this->rotation));
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), -this->position);
		this->viewMatrix = orientation * translation;
	}
	const glm::mat4& PerspectiveCamera::GetProjectionMatrix() const
	{
		return this->projectionMatrix;
	}
	const glm::mat4& PerspectiveCamera::GetViewMatrix() const
	{
		return this->viewMatrix;
	}
	const glm::mat4 PerspectiveCamera::GetViewProjectionMatrix() const
	{
		return this->projectionMatrix * this->viewMatrix;
	}
}
