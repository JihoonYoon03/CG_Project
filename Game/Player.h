#pragma once
#include "pch.h"
#include "Engine/Object.h"
#include "Engine/Camera.h"

class Scene;

// 로컬 입력을 게임 명령으로 바꾼 값입니다. 소켓 메시지를 해석한 뒤에도 이 경로를 사용합니다.
// 패킷으로 구조체 메모리를 그대로 전송하지 말고 필드별 직렬화와 서버 검증을 추가해야 합니다.
struct PlayerCommand
{
    glm::vec2 movement{0.0f};  // X: 좌우, Y: 전후 (전진은 -1)
    glm::vec2 lookDelta{0.0f}; // 마우스 이동량
    bool fire = false;
};

class Player : public Object
{
public:
    using Object::Object;
    void ApplyCommand(const PlayerCommand& command, float deltaTime, const Scene& scene);
    void UpdateCamera(Camera& camera) const;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float moveSpeed = 10.0f / 3.6f;
    float sensitivity = 0.2f;

private:
    void MoveAxis(int axis, float distance, const Scene& scene);
};
