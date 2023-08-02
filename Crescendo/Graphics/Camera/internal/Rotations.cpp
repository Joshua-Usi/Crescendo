#include "Rotations.hpp"

#include <cmath>
#include "glm/gtc/constants.hpp"

namespace Crescendo
{
    glm::quat EulerToQuaternion(const glm::vec3& euler)
    {
        glm::quat quaternion;
        float cy = std::cos(euler.z * 0.5f);
        float sy = std::sin(euler.z * 0.5f);
        float cp = std::cos(euler.x * 0.5f);
        float sp = std::sin(euler.x * 0.5f);
        float cr = std::cos(euler.y * 0.5f);
        float sr = std::sin(euler.y * 0.5f);

        quaternion.w = cr * cp * cy + sr * sp * sy;
        quaternion.x = sr * cp * cy - cr * sp * sy;
        quaternion.y = cr * sp * cy + sr * cp * sy;
        quaternion.z = cr * cp * sy - sr * sp * cy;

        return quaternion;
    }
    glm::vec3 QuaternionToEuler(const glm::quat& q)
    {
        glm::vec3 euler;
        float test = q.x * q.y + q.z * q.w;
        if (test > 0.499f)
        {
            // singularity at north pole
            euler.x = 2.0f * std::atan2(q.x, q.w);
            euler.z = glm::half_pi<float>();
            euler.y = 0.0f;
        }
        else if (test < -0.499f)
        {
            // singularity at south pole
            euler.x = -2.0f * std::atan2(q.x, q.w);
            euler.z = -glm::half_pi<float>();
            euler.y = 0.0f;
        }
        else
        {
            float sqx = q.x * q.x;
            float sqy = q.y * q.y;
            float sqz = q.z * q.z;
            euler.x = std::atan2(2.0f * q.y * q.w - 2.0f * q.x * q.z, 1.0f - 2.0f * sqy - 2.0f * sqz);
            euler.z = std::asin(2.0f * test);
            euler.y = std::atan2(2.0f * q.x * q.w - 2.0f * q.y * q.z, 1.0f - 2.0f * sqx - 2.0f * sqz);
        }
        return euler;
    }
    glm::quat Slerp(float t, const glm::quat& q1, const glm::quat& q2)
    {
        return glm::slerp(q1, q2, t);
    }
}