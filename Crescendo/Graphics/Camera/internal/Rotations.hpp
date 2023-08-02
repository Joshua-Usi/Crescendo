#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


namespace Crescendo
{
    /// <summary>
    /// Converts an Euler angle vector to a quaternion
    /// </summary>
    /// <param name="euler">vec3 euler angles in order: yaw, pitch, roll</param>
    /// <returns>Quaternion representation of rotation</returns>
    glm::quat EulerToQuaternion(const glm::vec3& euler);

    /// <summary>
    /// Converts a quaternion to an euler representation
    /// </summary>
    /// <param name="q">Quaternion to convert</param>
    /// <returns>Euler vec3 in order: yaw, pitch, roll</returns>
    glm::vec3 QuaternionToEuler(const glm::quat& q);
    /// <summary>
    /// Spherical linear interpolate between 2 quaternions
    /// </summary>
    /// <param name="t">time value between [0.0f, 1.0f]</param>
    /// <param name="q1">starting quaternion</param>
    /// <param name="q2">ending quaternion</param>
    /// <returns>linearly interpolated quaternion</returns>
    glm::quat Slerp(float t, const glm::quat& q1, const glm::quat& q2);
}