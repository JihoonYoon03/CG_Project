#include "pch.h"
#include "Player.h"
#include "Scene.h"

void Player::ApplyCommand(const PlayerCommand& command, float deltaTime, const Scene& scene)
{
	//[윤지훈] 나머지 연산으로 yaw값 -360~360 제한
	yaw = std::fmod(yaw - command.lookDelta.x * sensitivity, 360.0f);
	//[윤지훈] pitch는 추후 계산을 위해 -89 ~ +89 사이로 제한
	pitch = std::clamp(pitch - command.lookDelta.y * sensitivity, -89.0f, 89.0f);
	glm::vec2 movement = command.movement;

	//[윤지훈] 길이 1 이상인 경우 = 대각선 이동
	//대각 이동 속도 보정을 위해 길이 1로 정규화
	if (glm::length(movement) > 1.0f)
	{
		movement = glm::normalize(movement);
	}

	//[윤지훈] yaw값을 통한 y축기준 회전량을 쿼터니언으로 변환, 그걸 행렬로 만듦
	const auto rotation = glm::mat4_cast(glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0)));
	//[윤지훈] xz평면 이동량 계산. 이동 방향 벡터에 yaw를 통한 회전행렬을 곱하고, 속도와 시간 반영해 계산
	const glm::vec3 displacement =
		glm::vec3(rotation * glm::vec4(movement.x, 0, movement.y, 0)) * moveSpeed * deltaTime;
	// 원본은 X/Z 중심 판정 후 이번 이동 전체를 취소합니다.

	//[윤지훈] 이동 방향 벡터가 0이 아닐때에만 이동
	if (movement != glm::vec2(0))
	{
		transform.position += displacement;
		lastDisplacement_ = displacement;
		eyePosition_ = transform.position + glm::vec3(0.0f, 0.75f, -0.4f);
		eyeInitialized_ = true;
	}

	for (const auto& object : scene.GetObjects())
	{
		//[윤지훈] 충돌 시 이동을 아예 취소함. 미끄러지도록 개선 필요
		if (object.get() != this && object->active && object->blocksMovement && object->collider == ColliderType::Box && Collision::OverlapsXZ(GetWorldBox(), object->GetWorldBox()))
		{
			transform.position -= lastDisplacement_;
		}
	}
}

//[윤지훈] 눈 위치를 중심으로 좌우 회전하는 행렬 반환
glm::mat4 Player::GetModelMatrix() const
{
	//[윤지훈] 눈 활성화시 눈 위치, 아니면 현재 위치에 고정값 더해서 눈 위치를 구함
	const glm::vec3 eye = eyeInitialized_ ? eyePosition_ : transform.position + glm::vec3(0.0f, 0.75f, -0.4f);
	//[윤지훈] 눈 위치를 기준으로 이동하는 행렬 만듦(회전 적용 후 원상 복구를 위해)
	glm::mat4 pivot = glm::translate(glm::mat4(1), eye);
	//[윤지훈] y축 회전 행렬 곱함. 실제 변환 순서는 오른쪽 -> 왼쪽임
	pivot *= glm::mat4_cast(glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0)));

	//[윤지훈] 오브젝트 자체 변환 행렬에 눈 위치와 y축회전 반영된 행렬을 곱함
	//pivot에 -eye만큼 이동하는 것을 적용해야 eye 위치를 원점으로 해서 회전 적용 가능. 이후 원상 복구
	return glm::translate(pivot, -eye) * transform.GetMatrix();
}

//[윤지훈] 카메라는 플레이어 회전과 눈 위치를 따라감
void Player::UpdateCamera(Camera& camera) const
{
	// 원본은 이동 후 카메라를 갱신하고 충돌을 처리했습니다. 그 관찰 순서를 유지합니다.
	camera.position = eyeInitialized_ ? eyePosition_ : transform.position + glm::vec3(0.0f, 0.75f, -0.4f);
	camera.yaw = yaw;
	camera.pitch = pitch;
}
