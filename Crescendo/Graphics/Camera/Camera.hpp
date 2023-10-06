#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Crescendo::Graphics
{
	namespace internal
	{
		glm::vec3 QuaternionToEuler(const glm::quat& q)
		{
			glm::vec3 euler;
			float test = q.x * q.y + q.z * q.w;
			if (test > 0.499f)
			{
				// singularity at north pole
				euler.y = 2.0f * std::atan2(q.x, q.w);
				euler.z = glm::half_pi<float>();
				euler.x = 0.0f;
			}
			else if (test < -0.499f)
			{
				// singularity at south pole
				euler.y = -2.0f * std::atan2(q.x, q.w);
				euler.z = -glm::half_pi<float>();
				euler.x = 0.0f;
			}
			else
			{
				float sqx = q.x * q.x;
				float sqy = q.y * q.y;
				float sqz = q.z * q.z;
				euler.y = std::atan2(2.0f * q.y * q.w - 2.0f * q.x * q.z, 1.0f - 2.0f * sqy - 2.0f * sqz);
				euler.z = std::asin(2.0f * test);
				euler.x = std::atan2(2.0f * q.x * q.w - 2.0f * q.y * q.z, 1.0f - 2.0f * sqx - 2.0f * sqz);
			}
			return euler;
		}
	}
	/// <summary>
	/// Pure data-only camera
	/// </summary>
	class Camera
	{
	protected:
		glm::quat rotation;
		glm::vec3 position;
	public:
		inline Camera(const glm::vec3& initialPosition = glm::vec3(0.0f, 0.0f, 0.0f), const glm::quat& initialRotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f)) :
			rotation(initialRotation), position(initialPosition) {}
		virtual ~Camera() = default;

		// Data generation
		inline glm::mat4 GetViewMatrix() const { return glm::mat4_cast(glm::inverse(this->rotation)) * glm::translate(glm::mat4(1.0f), -this->position); };
		virtual glm::mat4 GetProjectionMatrix() const = 0;

		inline glm::mat4 GetViewProjectionMatrix() const { return this->GetProjectionMatrix() * this->GetViewMatrix(); }
		inline glm::vec3 GetForwardVector() const { return this->rotation * glm::vec3(0.0f, 0.0f, -1.0f); };
		inline glm::vec3 GetRightVector() const { return this->rotation * glm::vec3(1.0f, 0.0f, 0.0f); };
		inline glm::vec3 GetUpVector() const { return this->rotation * glm::vec3(0.0f, 1.0f, 0.0f); };

		// Positional and Movement
		inline void SetPosition(const glm::vec3& newPosition) { this->position = newPosition; };
		inline void Move(const glm::vec3& offset) { this->position += offset; };
		inline void MoveAlongForward(float distance) { this->position += this->GetForwardVector() * distance; };
		inline void MoveAlongRight(float distance) { this->position += this->GetRightVector() * distance; };
		inline void MoveAlongUp(float distance) { this->position += this->GetUpVector() * distance; };
		inline void MoveDirection(const glm::quat& quaternion, float distance) { this->position += quaternion * glm::vec3(0.0f, 0.0f, -1.0f) * distance; };
		inline void MoveDirection(const glm::vec3& direction, float distance) { this->position += glm::normalize(direction) * distance; };

		inline glm::vec3 GetPosition() const { return this->position; };

		// Orientation and Rotation
		inline void SetRotation(const glm::quat& quaternion) { this->rotation = quaternion; };
		inline void Rotate(const glm::quat& quaternion) { this->rotation *= quaternion; };
		inline void SetRotation(const glm::vec3& euler) { this->rotation = glm::quat(euler); };
		inline void Rotate(const glm::vec3& euler) { this->rotation *= glm::quat(euler); };
		inline void LookAt(const glm::vec3& target)
		{
			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 forward = glm::normalize(target - position);
			if (glm::dot(forward, up) > 0.99f)
			{
				up = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			else if (glm::dot(forward, up) < -0.99f)
			{
				up = glm::vec3(-1.0f, 0.0f, 0.0f);
			}
			glm::mat4 viewMatrix = glm::lookAt(position, target, up);
			glm::mat3 rotationMatrix = glm::mat3(viewMatrix);
			rotationMatrix = glm::transpose(rotationMatrix);
			rotation = glm::quat_cast(rotationMatrix);
		}

		inline glm::quat GetQuaternionRotation() const { return this->rotation; };
		inline glm::vec3 GetRotation() const { return internal::QuaternionToEuler(this->rotation); };

		// Universal settings
		virtual void SetClipPlanes(const glm::vec2& clipPlanes) = 0;

		virtual glm::vec2 GetClipPlanes() const = 0;

		// Projection settings
		virtual void SetFOV(float fov) = 0;
		virtual void SetAspectRatio(float aspectRatio) = 0;

		virtual float GetFOV() const = 0;
		virtual float GetAspectRatio() const = 0;

		// Orthographic settings
		virtual void SetOrthoBounds(const glm::vec4& bounds) = 0; // left, right, top, bottom
		virtual void OrthoZoom(float factor) = 0;

		virtual glm::vec4 GetOrthoBounds() const = 0;
	};

	class PerspectiveCamera : public Camera
	{
	private:
		float fov, aspectRatio;
		glm::vec2 clipPlanes;
	public:
		inline PerspectiveCamera() : Camera(), fov(0.0f), aspectRatio(0.0f), clipPlanes(0.0f, 0.0f) {}
		inline PerspectiveCamera(const glm::vec3& initialPosition, const glm::quat& initialRotation, float fov, float aspectRatio, const glm::vec2& clipPlanes) :
			Camera(initialPosition, initialRotation), fov(fov), aspectRatio(aspectRatio), clipPlanes(clipPlanes) {}
		inline PerspectiveCamera(float fov, float aspectRatio, const glm::vec2& clipPlanes) :
			Camera(), fov(fov), aspectRatio(aspectRatio), clipPlanes(clipPlanes) {}
		virtual ~PerspectiveCamera() override final = default;

		inline virtual glm::mat4 GetProjectionMatrix() const final { return glm::perspective(this->fov, this->aspectRatio, this->clipPlanes.x, this->clipPlanes.y); };
		inline virtual void SetClipPlanes(const glm::vec2& clipPlanes) final { this->clipPlanes = clipPlanes; };
		inline virtual glm::vec2 GetClipPlanes() const final { return this->clipPlanes; };
		
		inline virtual void SetFOV(float fov) final { this->fov = fov; };
		inline virtual void SetAspectRatio(float aspectRatio) final { this->aspectRatio = aspectRatio; };
		inline virtual float GetFOV() const final { return this->fov; };
		inline virtual float GetAspectRatio() const final { return this->aspectRatio; };

		// Ortho methods are not supported for perspective cameras so they do nothing
		inline virtual void SetOrthoBounds(const glm::vec4& bounds) final {};
		inline virtual void OrthoZoom(float factor) final {};
		inline virtual glm::vec4 GetOrthoBounds() const final { return glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); };
	};

	class OrthographicCamera : public Camera
	{
	private:
		glm::vec4 bounds; // left, right, top, bottom
		glm::vec2 clipPlanes;
	public:
		inline OrthographicCamera() : Camera(), bounds(0.0f, 0.0f, 0.0f, 0.0f), clipPlanes(0.0f, 0.0f) {}
		inline OrthographicCamera(const glm::vec3& initialPosition, const glm::quat& initialRotation, const glm::vec4& bounds, const glm::vec2& clipPlanes) :
			Camera(initialPosition, initialRotation), bounds(bounds), clipPlanes(clipPlanes) {}
		inline OrthographicCamera(const glm::vec4& bounds, const glm::vec2& clipPlanes) :
			Camera(), bounds(bounds), clipPlanes(clipPlanes) {}
		virtual ~OrthographicCamera() override final = default;

		inline virtual glm::mat4 GetProjectionMatrix() const final { return glm::ortho(this->bounds.x, this->bounds.y, this->bounds.z, this->bounds.w, this->clipPlanes.x, this->clipPlanes.y); };
		inline virtual void SetClipPlanes(const glm::vec2& clipPlanes) final { this->clipPlanes = clipPlanes; };
		inline virtual glm::vec2 GetClipPlanes() const final { return this->clipPlanes; };

		// Perspective methods are not supported for orthographic cameras, so they do nothing
		inline virtual void SetFOV(float fov) final {};
		inline virtual void SetAspectRatio(float aspectRatio) final {};
		inline virtual float GetFOV() const final { return 0.0f; };
		inline virtual float GetAspectRatio() const final { return 0.0f; };

		inline virtual void SetOrthoBounds(const glm::vec4& bounds) final { this->bounds = bounds; };
		inline virtual void OrthoZoom(float factor) final { this->bounds *= factor; };
		inline virtual glm::vec4 GetOrthoBounds() const  final { return this->bounds; };
	};
}