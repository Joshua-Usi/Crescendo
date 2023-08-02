#include "Camera.hpp"
#include "internal/Rotations.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Crescendo::Graphics
{
	void Camera::RecalculateMatrices()
	{
		glm::mat4 orientation = glm::mat4_cast(glm::inverse(this->rotation));
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), -this->position);
		this->viewMatrix = orientation * translation;
	}
	Camera::Camera(float fieldOfView, float aspectRatio, const glm::vec2& frustumPlanes)
		: projectionMatrix(glm::perspective(fieldOfView, aspectRatio, frustumPlanes.x, frustumPlanes.y)),
		viewMatrix(glm::mat4(1.0f)),
		position(glm::vec3(0.0f, 0.0f, 0.0f)),
		rotation(glm::quat(0.0f, 0.0f, 0.0f, 1.0f))
	{}
	Camera::Camera(float left, float right, float top, float bottom, const glm::vec2& frustumPlanes)
		: projectionMatrix(glm::ortho(left, right, top, bottom, frustumPlanes.x, frustumPlanes.y)),
		viewMatrix(glm::mat4(1.0f)),
		position(glm::vec3(0.0f, 0.0f, 0.0f)),
		rotation(glm::quat(0.0f, 0.0f, 0.0f, 1.0f))
	{}
	const glm::vec3& Camera::GetPosition() const
	{
		return this->position;
	}
	void Camera::SetOrthographic(float left, float right, float top, float bottom, const glm::vec2& frustumPlanes)
	{
		this->projectionMatrix = glm::ortho(left, right, top, bottom, frustumPlanes.x, frustumPlanes.y);
	}
	void Camera::SetPerspective(float fieldOfView, float aspectRatio, const glm::vec2& frustumPlanes)
	{
		this->projectionMatrix = glm::perspective(fieldOfView, aspectRatio, frustumPlanes.x, frustumPlanes.y);
	}
	const glm::vec3 Camera::GetRotation() const
	{
		return QuaternionToEuler(this->rotation);
	}
	const glm::quat& Camera::GetQuaternionRotation() const
	{
		return this->rotation;
	}

	const glm::mat4& Camera::GetProjectionMatrix() const
	{
		return this->projectionMatrix;
	}
	const glm::mat4& Camera::GetViewMatrix() const
	{
		return this->viewMatrix;
	}
	const glm::mat4 Camera::GetViewProjectionMatrix() const
	{
		return this->projectionMatrix * this->viewMatrix;
	}

	void Camera::SetPosition(const glm::vec3& newPosition)
	{
		this->position = newPosition;
		this->RecalculateMatrices();
	}
	void Camera::MovePosition(const glm::vec3& offset)
	{
		this->position += offset;
		this->RecalculateMatrices();
	}
	void Camera::MovePosition(float x, float y, float z)
	{
		this->position += glm::vec3(x, y, z);
		this->RecalculateMatrices();
	}
	void Camera::SetRotation(const glm::vec3& euler)
	{
		this->rotation = EulerToQuaternion(euler);
		this->RecalculateMatrices();
	}
	void Camera::SetRotation(const glm::quat& quaternion)
	{
		this->rotation = quaternion;
		this->RecalculateMatrices();
	}
	void Camera::SetRotation(float angle)
	{
		this->rotation = EulerToQuaternion(glm::vec3(0.0f, 0.0f, angle));
		this->RecalculateMatrices();
	}
}