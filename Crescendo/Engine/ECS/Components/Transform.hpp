#pragma once

#include "common.hpp"
#include "Component.hpp"

#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
{
	namespace internal
	{
		inline cs_std::math::vec3 QuaternionToEuler(const cs_std::math::quat& q)
		{
			cs_std::math::vec3 euler;
			float test = q.x * q.y + q.z * q.w;
			if (test > 0.499f)
			{
				// singularity at north pole
				euler.y = 2.0f * std::atan2(q.x, q.w);
				euler.z = cs_std::math::half_pi<float>();
				euler.x = 0.0f;
			}
			else if (test < -0.499f)
			{
				// singularity at south pole
				euler.y = -2.0f * std::atan2(q.x, q.w);
				euler.z = -cs_std::math::half_pi<float>();
				euler.x = 0.0f;
			}
			else
			{
				float sqx = q.x * q.x, sqy = q.y * q.y, sqz = q.z * q.z;
				euler.y = std::atan2(2.0f * q.y * q.w - 2.0f * q.x * q.z, 1.0f - 2.0f * sqy - 2.0f * sqz);
				euler.z = std::asin(2.0f * test);
				euler.x = std::atan2(2.0f * q.x * q.w - 2.0f * q.y * q.z, 1.0f - 2.0f * sqx - 2.0f * sqz);
			}
			return euler;
		}
	}
	struct Transform : public Component
	{
		cs_std::math::quat rotation;
		cs_std::math::vec3 position;
		cs_std::math::vec3 scale;

		Transform() : rotation(0.0f, 0.0f, 0.0f, 1.0f), position(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f) {}
		Transform(const cs_std::math::vec3& position, const cs_std::math::quat& rotation, const cs_std::math::vec3& scale) : rotation(rotation), position(position), scale(scale) {}
		// No scale
		Transform(const cs_std::math::vec3& position, const cs_std::math::quat& rotation) : rotation(rotation), position(position), scale(1.0f, 1.0f, 1.0f) {}
		// No rotation, no scale
		Transform(const cs_std::math::vec3& position) : rotation(0.0f, 0.0f, 0.0f, 1.0f), position(position), scale(1.0f, 1.0f, 1.0f) {}
		// If given a matrix, extract the position, rotation, and scale from it
		Transform(const cs_std::math::mat4& matrix) : position(cs_std::math::vec3(matrix[3])), scale(cs_std::math::vec3(cs_std::math::length(cs_std::math::vec3(matrix[0])), cs_std::math::length(cs_std::math::vec3(matrix[1])), cs_std::math::length(cs_std::math::vec3(matrix[2]))))
		{
			glm::mat3 rotationMatrix;
			rotationMatrix[0] = cs_std::math::vec3(matrix[0]) / scale.x;
			rotationMatrix[1] = cs_std::math::vec3(matrix[1]) / scale.y;
			rotationMatrix[2] = cs_std::math::vec3(matrix[2]) / scale.z;
			rotation = cs_std::math::quat_cast(rotationMatrix);
		}

		cs_std::math::mat4 GetModelMatrix() const { return cs_std::math::translate(cs_std::math::mat4_cast(rotation), position) * cs_std::math::scale(cs_std::math::mat4(1.0f), scale); }
		cs_std::math::mat4 GetCameraViewMatrix() const { return cs_std::math::mat4_cast(cs_std::math::inverse(this->rotation)) * cs_std::math::translate(cs_std::math::mat4(1.0f), -this->position); }

		operator cs_std::math::mat4() { return GetModelMatrix(); }

		cs_std::math::vec3 GetForwardVector() const { return this->rotation * cs_std::math::vec3(0.0f, 0.0f, -1.0f); };
		cs_std::math::vec3 GetRightVector() const { return this->rotation * cs_std::math::vec3(1.0f, 0.0f, 0.0f); };
		cs_std::math::vec3 GetUpVector() const { return this->rotation * cs_std::math::vec3(0.0f, 1.0f, 0.0f); };

		void SetPosition(const cs_std::math::vec3& newPosition) { this->position = newPosition; };
		void Move(const cs_std::math::vec3& offset) { this->position += offset; };
		void MoveAlongForward(float distance) { this->position += this->GetForwardVector() * distance; };
		void MoveAlongRight(float distance) { this->position += this->GetRightVector() * distance; };
		void MoveAlongUp(float distance) { this->position += this->GetUpVector() * distance; };
		void MoveDirection(const cs_std::math::quat& quaternion, float distance) { this->position += quaternion * cs_std::math::vec3(0.0f, 0.0f, -1.0f) * distance; };
		void MoveDirection(const cs_std::math::vec3& direction, float distance) { this->position += cs_std::math::normalize(direction) * distance; };

		cs_std::math::vec3 GetPosition() const { return this->position; };

		void SetRotation(const cs_std::math::quat& quaternion) { this->rotation = quaternion; };
		void Rotate(const cs_std::math::quat& quaternion) { this->rotation *= quaternion; };
		void SetRotation(const cs_std::math::vec3& euler) { this->rotation = cs_std::math::quat(euler); };
		void Rotate(const cs_std::math::vec3& euler) { this->rotation *= cs_std::math::quat(euler); };
		void LookAt(const cs_std::math::vec3& target)
		{
			cs_std::math::vec3 up = cs_std::math::vec3(0.0f, 1.0f, 0.0f);
			cs_std::math::vec3 forward = cs_std::math::normalize(target - position);
			if (cs_std::math::dot(forward, up) > 0.99f)
			{
				up = cs_std::math::vec3(1.0f, 0.0f, 0.0f);
			}
			else if (cs_std::math::dot(forward, up) < -0.99f)
			{
				up = cs_std::math::vec3(-1.0f, 0.0f, 0.0f);
			}
			cs_std::math::mat4 viewMatrix = cs_std::math::lookAt(position, target, up);
			cs_std::math::mat3 rotationMatrix = cs_std::math::mat3(viewMatrix);
			rotationMatrix = cs_std::math::transpose(rotationMatrix);
			rotation = cs_std::math::quat_cast(rotationMatrix);
		}

		cs_std::math::quat GetQuaternionRotation() const { return this->rotation; };
		cs_std::math::vec3 GetRotation() const { return internal::QuaternionToEuler(this->rotation); };

		// Gets the direction as a normalised vector
		cs_std::math::vec3 GetDirection() const { return cs_std::math::normalize(glm::rotate(this->rotation, cs_std::math::vec3(0.0f, 0.0f, -1.0f))); };
	};
}