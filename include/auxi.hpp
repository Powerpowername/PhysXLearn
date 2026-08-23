#pragma once

#include "PxPhysicsAPI.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Learner
{
inline glm::vec3 toGlm(const physx::PxVec3& v)
{
    return glm::vec3(v.x, v.y, v.z);
}

inline physx::PxVec3 toPx(const glm::vec3& v)
{
    return physx::PxVec3(v.x, v.y, v.z);
}

inline glm::quat toGlm(const physx::PxQuat& q)
{
    return glm::quat(q.w, q.x, q.y, q.z);
}

inline physx::PxQuat toPx(const glm::quat& q)
{
    return physx::PxQuat(q.x, q.y, q.z, q.w);
}

inline glm::mat4 toGlm(const physx::PxTransform& t)
{
    glm::mat4 rotation = glm::toMat4(toGlm(t.q));
    rotation[3] = glm::vec4(toGlm(t.p), 1.0f);
    return rotation;
}

inline physx::PxTransform toPxTransform(const glm::vec3& position, const glm::quat& rotation)
{
    return physx::PxTransform(toPx(position), toPx(rotation));
}

inline physx::PxTransform toPxTransform(const glm::mat4& m)
{
    glm::vec3 position(m[3]);
    glm::quat rotation = glm::quat_cast(m);
    return toPxTransform(position, rotation);
}

inline glm::mat4 toGlm(const physx::PxMat44& m)
{
    return glm::mat4(
        glm::vec4(m.column0.x, m.column0.y, m.column0.z, m.column0.w),
        glm::vec4(m.column1.x, m.column1.y, m.column1.z, m.column1.w),
        glm::vec4(m.column2.x, m.column2.y, m.column2.z, m.column2.w),
        glm::vec4(m.column3.x, m.column3.y, m.column3.z, m.column3.w));
}

inline physx::PxMat44 toPxMat44(const glm::mat4& m)
{
    return physx::PxMat44(
        physx::PxVec4(m[0][0], m[0][1], m[0][2], m[0][3]),
        physx::PxVec4(m[1][0], m[1][1], m[1][2], m[1][3]),
        physx::PxVec4(m[2][0], m[2][1], m[2][2], m[2][3]),
        physx::PxVec4(m[3][0], m[3][1], m[3][2], m[3][3]));
}
} // namespace auxi
