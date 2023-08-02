#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Crescendo::Graphics
{
	class Camera
	{
	private:
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		// Quaternions! holy these are so much easier than euler
		// Defaults to facing -z direction
		glm::quat rotation;
		glm::vec3 position;
	private:
		void RecalculateMatrices();
	public:
		Camera() = default;
		/// <summary>
		/// Creates a perspective projecting camera
		/// </summary>
		/// <param name="fieldOfView"></param>
		/// <param name="aspectRatio"></param>
		/// <param name="nearPlane"></param>
		/// <param name="farPlane"></param>
		Camera(float fieldOfView, float aspectRatio, const glm::vec2& frustumPlanes = { 0.1f, 1000.0f });
		/// <summary>
		/// Creates an orthographic projecting camera
		/// </summary>
		/// <param name="left"></param>
		/// <param name="right"></param>
		/// <param name="top"></param>
		/// <param name="bottom"></param>
		/// <param name="nearPlane"></param>
		/// <param name="farPlane"></param>
		Camera(float left, float right, float top, float bottom, const glm::vec2& frustumPlanes = { 0.0f, 1000.0f });
		~Camera() = default;

		void SetOrthographic(float left, float right, float top, float bottom, const glm::vec2& frustumPlanes = { 0.0f, 1000.0f });
		void SetPerspective(float fieldOfView, float aspectRatio, const glm::vec2& frustumPlanes = { 0.1f, 1000.0f });

		/// <summary>
		/// Returns a reference to the position of the camera
		/// </summary>
		const glm::vec3& GetPosition() const;
		/// <summary>
		/// Get the rotation of the camera in euler directions
		/// In the order of yaw, pitch, roll
		/// </summary>
		const glm::vec3 GetRotation() const;
		/// <summary>
		/// Returns a reference to the quaternion rotation of the camera.
		/// </summary>
		const glm::quat& GetQuaternionRotation() const;

		/// <summary>
		/// Returns a reference to the projection matrix
		/// </summary>
		const glm::mat4& GetProjectionMatrix() const;
		/// <summary>
		/// Returns a reference to the view matrix
		/// </summary>
		const glm::mat4& GetViewMatrix() const;
		/// <summary>
		/// Returns a copy of the view-projection matrix
		/// </summary>
		const glm::mat4 GetViewProjectionMatrix() const;
		/// <summary>
		/// Set the position of the camera in 3 dimensions
		/// Usually used for perspective cameras
		/// </summary>
		/// <param name="newPosition"></param>
		void SetPosition(const glm::vec3& newPosition);
		/// <summary>
		/// Move the position of the camera in 3 dimensions
		/// </summary>
		/// <param name="offset">vector for offset</param>
		void MovePosition(const glm::vec3& offset);
		/// <summary>
		/// Move the position of the camera in 3 dimensions
		/// </summary>
		/// <param name="x">X offset</param>
		/// <param name="y">Y offset</param>
		/// <param name="z">Z offset</param>
		void MovePosition(float x, float y, float z);
		/// <summary>
		/// Set the rotation of the camera using euler directions
		/// Usually used for perspective cameras
		/// Vector is in order: yaw, pitch, roll
		/// </summary>
		/// <param name="euler">Euler directions in the order yaw, pitch, roll</param>
		void SetRotation(const glm::vec3& euler);
		/// <summary>
		/// Set the rotation of the camera using quaternions
		/// Usually used for perspective cameras
		/// </summary>
		/// <param name="quaternion">Quaternions rotation</param>
		void SetRotation(const glm::quat& quaternion);
		/// <summary>
		/// Set the rotation of the camera in the z-axis in a clockwise direction
		/// Usually used for orthographic cameras
		/// </summary>
		/// <param name="angle">Angle of rotation in radians</param>
		void SetRotation(float angle);
	};
}