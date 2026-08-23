#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

enum CameraMove
{
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

class Camera
{
private:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float moveSpeed = 5.0f;
    float mouseSensitivity = 0.1f;
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    inline float clampFloat(float value, float minValue, float maxValue) const
    {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }

    inline void updateRotation()
    {
        glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        rotation = glm::normalize(yawQuat * pitchQuat);
    }

public:
    Camera() = default;

    Camera(const glm::vec3& position, float yaw = 0.0f, float pitch = 0.0f)
        : position(position), yaw(yaw), pitch(pitch)
    {
        updateRotation();
    }

    inline glm::mat4 getViewMatrix() const
    {
        glm::mat4 rotate = glm::mat4_cast(glm::conjugate(rotation));
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), -position);
        return rotate * translate;
    }

    inline glm::mat4 getProjectionMatrix(float aspect) const
    {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    inline glm::vec3 getPosition() const
    {
        return position;
    }

    inline glm::quat getRotation() const
    {
        return rotation;
    }

    inline glm::vec3 getFront() const
    {
        return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    inline glm::vec3 getRight() const
    {
        return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    inline glm::vec3 getUp() const
    {
        return glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    inline float getFov() const
    {
        return fov;
    }

    inline void setPosition(const glm::vec3& value)
    {
        position = value;
    }

    inline void setRotation(const glm::quat& value)
    {
        rotation = glm::normalize(value);
    }

    inline void setSpeed(float value)
    {
        moveSpeed = value;
    }

    inline void setMouseSensitivity(float value)
    {
        mouseSensitivity = value;
    }

    inline void setClip(float nearValue, float farValue)
    {
        nearPlane = nearValue;
        farPlane = farValue;
    }

    inline void processKeyboard(CameraMove direction, float deltaTime)
    {
        float velocity = moveSpeed * deltaTime;

        if (direction == CAMERA_FORWARD) position += getFront() * velocity;
        if (direction == CAMERA_BACKWARD) position -= getFront() * velocity;
        if (direction == CAMERA_LEFT) position -= getRight() * velocity;
        if (direction == CAMERA_RIGHT) position += getRight() * velocity;
        if (direction == CAMERA_UP) position += getUp() * velocity;
        if (direction == CAMERA_DOWN) position -= getUp() * velocity;
    }

    inline void processMouse(float xoffset, float yoffset, bool constrainPitch = true)
    {
        yaw -= xoffset * mouseSensitivity;
        pitch += yoffset * mouseSensitivity;

        if (constrainPitch)
        {
            pitch = clampFloat(pitch, -89.0f, 89.0f);
        }

        updateRotation();
    }

    inline void processScroll(float yoffset)
    {
        fov -= yoffset;
        fov = clampFloat(fov, 1.0f, 90.0f);
    }

    inline void lookAt(const glm::vec3& target)
    {
        glm::vec3 direction = glm::normalize(target - position);
        pitch = glm::degrees(glm::asin(clampFloat(direction.y, -1.0f, 1.0f)));
        yaw = glm::degrees(glm::atan(-direction.x, -direction.z));
        updateRotation();
    }
};

