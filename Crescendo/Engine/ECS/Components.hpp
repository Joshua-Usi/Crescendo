#pragma once

#include "common.hpp"

#include <string>

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"

namespace CrescendoEngine
{
	namespace internal
	{
		cs_std::math::vec3 QuaternionToEuler(const cs_std::math::quat& q)
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

	struct Behaviour
	{
		virtual void OnUpdate(double time) {}
		virtual void OnLateUpdate(double time) {}
		virtual void OnAttach(Entity e) {}
		virtual void OnDetach(Entity e) {}
	};

	struct Name : public Component
	{
		std::string name;
		Name(const std::string& name) : name(name) {}
		Name(const char* name) : name(name) {}
	};

	struct Transform : public Component
	{
		cs_std::math::quat rotation;
		cs_std::math::vec3 position;
		cs_std::math::vec3 scale;

		Transform() : rotation(0.0f, 0.0f, 0.0f, 1.0f), position(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f) {}
		Transform(const cs_std::math::vec3& position, const cs_std::math::quat& rotation, const cs_std::math::vec3& scale) : rotation(rotation), position(position), scale(scale) {}
		// No scale
		Transform(const cs_std::math::vec3& position, const cs_std::math::quat& rotation) : rotation(rotation), position(position), scale(1.0f, 1.0f, 1.0f) {}
		// If given a matrix, extract the position, rotation, and scale from it
		Transform(const cs_std::math::mat4& matrix) : rotation(cs_std::math::quat_cast(matrix)), position(matrix[3]), scale(cs_std::math::vec3(cs_std::math::length(matrix[0]), cs_std::math::length(matrix[1]), cs_std::math::length(matrix[2]))) {}

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
	};

	struct Material : public Component
	{
		uint32_t pipelineID, diffuseID, normalID;
		bool isTransparent, isDoubleSided, isShadowCasting;

		Material() : pipelineID(0), diffuseID(0), normalID(0), isTransparent(false), isDoubleSided(false), isShadowCasting(true) {}
		Material(uint32_t pipelineID, uint32_t diffuseID, uint32_t normalID, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipelineID(pipelineID), diffuseID(diffuseID), normalID(normalID), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};

	struct MeshData : public Component
	{
		cs_std::graphics::bounding_aabb bounds;
		uint32_t meshID;

		MeshData() : bounds(), meshID(0) {}
		MeshData(const cs_std::graphics::bounding_aabb& bounds, uint32_t meshID) : bounds(bounds), meshID(meshID) {}
	};

	struct Skybox : public Component
	{
		uint32_t meshID, textureID;

		Skybox() : meshID(std::numeric_limits<uint32_t>::max()), textureID(std::numeric_limits<uint32_t>::max()) {}
		Skybox(uint32_t meshID, uint32_t textureID) : meshID(meshID), textureID(textureID) {}
	};


	struct OrthographicCamera : public Component
	{
		float left, right, bottom, top, nearPlane, farPlane;

		OrthographicCamera() : left(-1.0f), right(1.0f), bottom(-1.0f), top(1.0f), nearPlane(0.1f), farPlane(1000.0f) {}
		OrthographicCamera(float left, float right, float bottom, float top, float nearPlane, float farPlane) : left(left), right(right), bottom(bottom), top(top), nearPlane(nearPlane), farPlane(farPlane) {}
	
		cs_std::math::mat4 GetProjectionMatrix() const { return cs_std::math::ortho(left, right, bottom, top, nearPlane, farPlane); }
		cs_std::math::mat4 GetViewProjectionMatrix(const cs_std::math::mat4& viewMatrix) const { return GetProjectionMatrix() * viewMatrix; }
	};
	struct PerspectiveCamera : public Component
	{
		float fov, aspectRatio, nearPlane, farPlane;
		PerspectiveCamera() : fov(45.0f), aspectRatio(16.0f / 9.0f), nearPlane(0.1f), farPlane(1000.0f) {}
		PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane) : fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {}

		// We use reverse-Z so near and far planes are flipped
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

	struct Behaviours : public Component
	{
		std::vector<std::shared_ptr<Behaviour>> behaviours;

		Behaviours() : behaviours() {}
		Behaviours(std::vector<std::shared_ptr<Behaviour>> behaviours) : behaviours(behaviours) {}
		template<typename ...T> Behaviours(T... behaviours) : behaviours({ behaviours... }) {}

		Behaviours& AddBehaviour(std::shared_ptr<Behaviour> behaviour) { behaviours.push_back(behaviour); return *this; }
		Behaviours& AddBehaviour(Behaviour* behaviour) { behaviours.push_back(std::shared_ptr<Behaviour>(behaviour)); return *this; }

		// Pre-render
		void OnUpdate(double time) { for (auto& behaviour : behaviours) behaviour->OnUpdate(time); }
		// Post-render
		void OnLateUpdate(double time) { for (auto& behaviour : behaviours) behaviour->OnLateUpdate(time); }
		void OnAttach(Entity e) { for (auto& behaviour : behaviours) behaviour->OnAttach(e); }
		void OnDetach(Entity e) { for (auto& behaviour : behaviours) behaviour->OnDetach(e); }
	};
}