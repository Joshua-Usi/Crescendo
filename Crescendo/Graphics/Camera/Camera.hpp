#pragma once

#include "common.hpp"

#include "cs_std/math/math.hpp"

CS_NAMESPACE_BEGIN
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
	class Camera
	{
	protected:
		cs_std::math::quat rotation;
		cs_std::math::vec3 position;
	public:
		Camera(const cs_std::math::vec3& initialPosition = cs_std::math::vec3(0.0f, 0.0f, 0.0f), const cs_std::math::quat& initialRotation = cs_std::math::quat(0.0f, 0.0f, 0.0f, 1.0f)) :
			rotation(initialRotation), position(initialPosition) {}
		virtual ~Camera() = default;

		// Data generation
		cs_std::math::mat4 GetViewMatrix() const { return cs_std::math::mat4_cast(cs_std::math::inverse(this->rotation)) * cs_std::math::translate(cs_std::math::mat4(1.0f), -this->position); };
		virtual cs_std::math::mat4 GetProjectionMatrix() const = 0;

		cs_std::math::mat4 GetViewProjectionMatrix() const { return this->GetProjectionMatrix() * this->GetViewMatrix(); }
		cs_std::math::vec3 GetForwardVector() const { return this->rotation * cs_std::math::vec3(0.0f, 0.0f, -1.0f); };
		cs_std::math::vec3 GetRightVector() const { return this->rotation * cs_std::math::vec3(1.0f, 0.0f, 0.0f); };
		cs_std::math::vec3 GetUpVector() const { return this->rotation * cs_std::math::vec3(0.0f, 1.0f, 0.0f); };

		// Positional and Movement
		void SetPosition(const cs_std::math::vec3& newPosition) { this->position = newPosition; };
		void Move(const cs_std::math::vec3& offset) { this->position += offset; };
		void MoveAlongForward(float distance) { this->position += this->GetForwardVector() * distance; };
		void MoveAlongRight(float distance) { this->position += this->GetRightVector() * distance; };
		void MoveAlongUp(float distance) { this->position += this->GetUpVector() * distance; };
		void MoveDirection(const cs_std::math::quat& quaternion, float distance) { this->position += quaternion * cs_std::math::vec3(0.0f, 0.0f, -1.0f) * distance; };
		void MoveDirection(const cs_std::math::vec3& direction, float distance) { this->position += cs_std::math::normalize(direction) * distance; };

		cs_std::math::vec3 GetPosition() const { return this->position; };

		// Orientation and Rotation
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

		// Universal settings
		virtual void SetClipPlanes(const cs_std::math::vec2& clipPlanes) = 0;

		virtual cs_std::math::vec2 GetClipPlanes() const = 0;

		// Projection settings
		virtual void SetFOV(float fov) = 0;
		virtual void SetAspectRatio(float aspectRatio) = 0;

		virtual float GetFOV() const = 0;
		virtual float GetAspectRatio() const = 0;

		// Orthographic settings
		virtual void SetOrthoBounds(const cs_std::math::vec4& bounds) = 0; // left, right, top, bottom
		virtual void OrthoZoom(float factor) = 0;

		virtual cs_std::math::vec4 GetOrthoBounds() const = 0;
	};

	class PerspectiveCamera : public Camera
	{
	private:
		float fov, aspectRatio;
		cs_std::math::vec2 clipPlanes;
	public:
		PerspectiveCamera() : Camera(), fov(0.0f), aspectRatio(0.0f), clipPlanes(0.0f, 0.0f) {}
		PerspectiveCamera(const cs_std::math::vec3& initialPosition, const cs_std::math::quat& initialRotation, float fov, float aspectRatio, const cs_std::math::vec2& clipPlanes) :
			Camera(initialPosition, initialRotation), fov(fov), aspectRatio(aspectRatio), clipPlanes(clipPlanes) {}
		PerspectiveCamera(float fov, float aspectRatio, const cs_std::math::vec2& clipPlanes) :
			Camera(), fov(fov), aspectRatio(aspectRatio), clipPlanes(clipPlanes) {}
		virtual ~PerspectiveCamera() override final = default;

		virtual cs_std::math::mat4 GetProjectionMatrix() const final { return cs_std::math::perspective(this->fov, this->aspectRatio, this->clipPlanes.x, this->clipPlanes.y); };
		virtual void SetClipPlanes(const cs_std::math::vec2& clipPlanes) final { this->clipPlanes = clipPlanes; };
		virtual cs_std::math::vec2 GetClipPlanes() const final { return this->clipPlanes; };

		virtual void SetFOV(float fov) final { this->fov = fov; };
		virtual void SetAspectRatio(float aspectRatio) final { this->aspectRatio = aspectRatio; };
		virtual float GetFOV() const final { return this->fov; };
		virtual float GetAspectRatio() const final { return this->aspectRatio; };

		// Ortho methods are not supported for perspective cameras so they do nothing
		virtual void SetOrthoBounds(const cs_std::math::vec4& bounds) final {};
		virtual void OrthoZoom(float factor) final {};
		virtual cs_std::math::vec4 GetOrthoBounds() const final { return cs_std::math::vec4(0.0f, 0.0f, 0.0f, 0.0f); };
	};

	class OrthographicCamera : public Camera
	{
	private:
		cs_std::math::vec4 bounds; // left, right, top, bottom
		cs_std::math::vec2 clipPlanes;
	public:
		OrthographicCamera() : Camera(), bounds(0.0f, 0.0f, 0.0f, 0.0f), clipPlanes(0.0f, 0.0f) {}
		OrthographicCamera(const cs_std::math::vec3& initialPosition, const cs_std::math::quat& initialRotation, const cs_std::math::vec4& bounds, const cs_std::math::vec2& clipPlanes) :
			Camera(initialPosition, initialRotation), bounds(bounds), clipPlanes(clipPlanes) {}
		OrthographicCamera(const cs_std::math::vec4& bounds, const cs_std::math::vec2& clipPlanes) :
			Camera(), bounds(bounds), clipPlanes(clipPlanes) {}
		virtual ~OrthographicCamera() override final = default;

		virtual cs_std::math::mat4 GetProjectionMatrix() const final { return cs_std::math::ortho(this->bounds.x, this->bounds.y, this->bounds.z, this->bounds.w, this->clipPlanes.x, this->clipPlanes.y); };
		virtual void SetClipPlanes(const cs_std::math::vec2& clipPlanes) final { this->clipPlanes = clipPlanes; };
		virtual cs_std::math::vec2 GetClipPlanes() const final { return this->clipPlanes; };

		// Perspective methods are not supported for orthographic cameras, so they do nothing
		virtual void SetFOV(float fov) final {};
		virtual void SetAspectRatio(float aspectRatio) final {};
		virtual float GetFOV() const final { return 0.0f; };
		virtual float GetAspectRatio() const final { return 0.0f; };

		virtual void SetOrthoBounds(const cs_std::math::vec4& bounds) final { this->bounds = bounds; };
		virtual void OrthoZoom(float factor) final { this->bounds *= factor; };
		virtual cs_std::math::vec4 GetOrthoBounds() const final { return this->bounds; };
	};
}