#include "pch.h"
#include "Camera.h"

glm::mat4 Camera::GetRotation() const
{
    const auto yawMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
    return glm::rotate(yawMatrix, glm::radians(pitch), glm::vec3(1, 0, 0));
}

glm::vec3 Camera::GetForward() const
{
    return glm::vec3(GetRotation() * glm::vec4(0, 0, -1, 0));
}

glm::mat4 Camera::GetView() const
{
    return glm::lookAt(position, position + GetForward(), glm::vec3(0, 1, 0));
}

glm::mat4 Camera::GetProjection(float aspect) const
{
    return glm::perspective(glm::radians(fieldOfView), aspect, nearPlane, farPlane);
}
