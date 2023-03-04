#pragma once

#include "cmath"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/constants.hpp"


namespace Crescendo::Mathematics
{
    /// <summary>
    /// Converts an Euler angle vector to a quaternion
    /// </summary>
    /// <param name="euler">vec3 euler angles in order: yaw, pitch, roll</param>
    /// <returns>Quaternion representation of rotation</returns>
    glm::quat EulerToQuaternion(const glm::vec3& euler) {
        glm::quat quaternion;
        float cy = cos(euler.z * 0.5f);
        float sy = sin(euler.z * 0.5f);
        float cp = cos(euler.x * 0.5f);
        float sp = sin(euler.x * 0.5f);
        float cr = cos(euler.y * 0.5f);
        float sr = sin(euler.y * 0.5f);

        quaternion.w = cr * cp * cy + sr * sp * sy;
        quaternion.x = sr * cp * cy - cr * sp * sy;
        quaternion.y = cr * sp * cy + sr * cp * sy;
        quaternion.z = cr * cp * sy - sr * sp * cy;

        return quaternion;
    }

    /// <summary>
    /// Converts a quaternion to an euler representation
    /// </summary>
    /// <param name="q">Quaternion to convert</param>
    /// <returns>Euler vec3 in order: yaw, pitch, roll</returns>
    glm::vec3 QuaternionToEuler(const glm::quat& q) {
        glm::vec3 euler;
        float test = q.x * q.y + q.z * q.w;
        if (test > 0.499f) {
            // singularity at north pole
            euler.x = 2.0f * atan2(q.x, q.w);
            euler.z = glm::half_pi<float>();
            euler.y = 0.0f;
        }
        else if (test < -0.499f) {
            // singularity at south pole
            euler.x = -2.0f * atan2(q.x, q.w);
            euler.z = -glm::half_pi<float>();
            euler.y = 0.0f;
        }
        else {
            float sqx = q.x * q.x;
            float sqy = q.y * q.y;
            float sqz = q.z * q.z;
            euler.x = atan2(2.0f * q.y * q.w - 2.0f * q.x * q.z, 1.0f - 2.0f * sqy - 2.0f * sqz);
            euler.z = asin(2.0f * test);
            euler.y = atan2(2.0f * q.x * q.w - 2.0f * q.y * q.z, 1.0f - 2.0f * sqx - 2.0f * sqz);
        }
        return euler;
    }
    /// <summary>
    /// Spherical linear interpolate between 2 quaternions
    /// </summary>
    /// <param name="t">time value between [0.0f, 1.0f]</param>
    /// <param name="q1">starting quaternion</param>
    /// <param name="q2">ending quaternion</param>
    /// <returns>linearly interpolated quaternion</returns>
    glm::quat Slerp(float t, const glm::quat& q1, const glm::quat& q2)
    {
        return glm::slerp(q1, q2, t);
    }
}