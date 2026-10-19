#include "pch.h"
#include "Gun.h"

void Gun::Fire()
{
	recoiling_ = true;
	recoilTime_ = 0.0f;
	recoilAmount_ = 0.0f;
}

void Gun::Update(float deltaTime)
{
	// Character.cpp의 키프레임을 먼저 평가한 뒤 시간을 증가시키는 순서를 보존합니다.
	recoilAmount_ = 0.0f;

	if (recoiling_)
	{
		//[윤지훈] 반동중일 떼, 0.05초 지나기 전이면 발사 후 경과 시간 / 0.05초로 최대 1까지 반동 반영
		// 그렇지 않다면 (1 - 발사 후 경과 시간(0.05초 이상)) / 0.95초로 1에서 0으로 수치 변화하도록 하는 수식
		recoilAmount_ = recoilTime_ < 0.05f ? recoilTime_ / 0.05f : (1.0f - recoilTime_) / 0.95f;
		//[윤지훈] 반동 경과 시간은 현재 경과시간 + 델타타임의 1.5배수로 계산 << 개선 필요?
		recoilTime_ = std::min(1.0f, recoilTime_ + deltaTime * 1.5f);

		if (recoilTime_ >= 1.0f)
		{
			recoiling_ = false;
			recoilTime_ = 0.0f;
		}
	}
}

void Gun::FollowCamera(const Camera& camera)
{
	//[윤지훈] 플레이어 위치 인자로 카메라 위치를 넣는데,
	// 이유는 현재 카메라가 플레이어 위치에 종속되어 갱신되고 있음.
	FollowCamera(camera, camera.position - glm::vec3(0.0f, 0.75f, -0.4f));
}

void Gun::FollowCamera(const Camera& camera, const glm::vec3& playerPosition)
{
	// 부모 이동과 카메라 중심 회전을 별도로 합성합니다. 충돌 직후에도 같은 값을 가정하지 않습니다.
	//[윤지훈] 총 위치는 플레이어 기준 상대 위치(=카메라 위치)임.
	transform.position = playerPosition + glm::vec3(0.2f, 0.35f, -0.8f);
	//[윤지훈] 모델 원본이 뒤집혀있어서 180도 회전
	transform.rotation = {0.0f, 180.0f, 0.0f};
	//[윤지훈] 카메라피벗 행렬은 월드 변환행렬 마지막에 곱해짐. 카메라 회전 따라가는 용도
	cameraPivot_ = glm::translate(glm::mat4(1), camera.position) * camera.GetRotation() *
				   glm::translate(glm::mat4(1), -camera.position);
}

glm::mat4 Gun::GetModelMatrix() const
{
	//[윤지훈] 최대 반동 시의 상대 위치에 반동 수치(0~1)와 스케일을 나눔(이동량도 작아지는 것을 보정)
	const glm::vec3 localPosition = glm::vec3(0.0f, 0.1f, -0.2f) * recoilAmount_ / transform.scale;
	//[윤지훈] 최대 반동 시에 X축 회전량에 반동 수치를 곱해 계산
	const auto pitchRotation = glm::angleAxis(glm::radians(-15.0f * recoilAmount_), glm::vec3(1, 0, 0));
	//[윤지훈] 쿼터니언으로 표현된 회전량을 행렬로 변환
	glm::mat4 animation = glm::mat4_cast(pitchRotation);
	//[윤지훈] 이동 연산도 결합하여 애니메이션 행렬 완성
	animation = glm::translate(animation, localPosition);

	//[윤지훈] 애니메이션 행렬 + 변환 행렬 + 카메라 기준 회전 행렬로 월드 행렬 완성
	return cameraPivot_ * transform.GetMatrix() * animation;
}
