#include "pch.h"
#include "Player.h"
#include "Engine/Scene.h"

void Player::ApplyCommand(const PlayerCommand& command, float deltaTime, const Scene& scene)
{
    yaw = std::fmod(yaw - command.lookDelta.x * sensitivity, 360.0f);
    pitch = std::clamp(pitch - command.lookDelta.y * sensitivity, -89.0f, 89.0f);
    glm::vec2 movement = command.movement;
    if (glm::length(movement) > 1.0f)
    {
        movement = glm::normalize(movement);
    }
    const auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0, 1, 0));
    const glm::vec3 displacement =
        glm::vec3(rotation * glm::vec4(movement.x, 0, movement.y, 0)) * moveSpeed * deltaTime;
    // 축별로 검사하므로 벽에 닿아도 벽을 따라 이동할 수 있습니다.
    MoveAxis(0, displacement.x, scene);
    MoveAxis(2, displacement.z, scene);
}

void Player::MoveAxis(int axis, float distance, const Scene& scene)
{
    if (distance == 0.0f)
    {
        return;
    }
    const float previous = transform.position[axis];
    transform.position[axis] += distance;
    const Box box = GetWorldBox();
    for (const auto& object : scene.GetObjects())
    {
        if (object.get() == this || !object->active || !object->blocksMovement)
        {
            continue;
        }
        if (object->collider == ColliderType::Box && Collision::Overlaps(box, object->GetWorldBox()))
        {
            transform.position[axis] = previous;
            break;
        }
    }
}

void Player::UpdateCamera(Camera& camera) const
{
    camera.position = transform.position + glm::vec3(0.0f, 0.75f, -0.4f);
    camera.yaw = yaw;
    camera.pitch = pitch;
}
