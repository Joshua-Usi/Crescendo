#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Crescendo::Rendering
{
	class PerspectiveCamera
	{
	private:
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;

		glm::vec3 position;
		// Quaternions! holy these are so much easier than euler
		// Defaults to facing -z direction
		glm::quat rotation;
	private:
		void RecalculateMatrices();
	public:
		PerspectiveCamera(float fieldOfView, float aspectRatio, float nearPlane = 1.0f, float farPlane = 1000.0f);

		const glm::vec3& GetPosition() const;
		const glm::vec3 GetRotation() const;
		const glm::quat& GetQuaternionRotation() const;
		const glm::mat4& GetProjectionMatrix() const;
		const glm::mat4& GetViewMatrix() const;
		const glm::mat4 GetViewProjectionMatrix() const;

		void SetPosition(const glm::vec3& newPosition);
		/// <summary>
		/// Vector is in order: yaw, pitch, roll
		/// </summary>
		/// <param name="quaternion"></param>
		void SetRotation(const glm::vec3& euler);
		void SetRotation(const glm::quat& quaternion);
	};
}