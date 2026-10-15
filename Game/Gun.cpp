#include "pch.h"
#include "Gun.h"

void Gun::Update(float deltaTime)
{
    recoilTime_ = std::min(1.0f, recoilTime_ + deltaTime * 1.5f);
}

void Gun::FollowCamera(const Camera& camera)
{
    const float amount = recoilTime_ < 0.05f ? recoilTime_ / 0.05f : (1.0f - recoilTime_) / 0.95f;
    const auto offset = glm::vec3(0.2f, -0.4f, -0.8f) + glm::vec3(0.0f, 0.1f, 0.2f) * amount;
    transform.position = camera.position + glm::vec3(camera.GetRotation() * glm::vec4(offset, 0.0f));
    transform.rotation = glm::vec3(-camera.pitch + 15.0f * amount, camera.yaw + 180.0f, 0.0f);
}
