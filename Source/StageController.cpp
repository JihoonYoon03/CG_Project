#include "pch.h"
#include "StageController.h"

bool StageController::Start(int stageNumber)
{
	//[윤지훈] 스테이지 진행이 끝났거나 클리어 상태가 아니면 다른 스테이지 시작 불가능
	if (state_ != StageState::Idle && state_ != StageState::Clear) return false;

	//[윤지훈] 지정된 스테이지 범위를 벗어나는 경우는 실행 불가
	if (stageNumber < 1 || stageNumber > static_cast<int>(stages_.size())) return false;

	//[윤지훈] 스테이지 관련 수치들 전부 초기화
	ResetValues(stageNumber);
	ruleIndex_ = stageIndex_;
	state_ = StageState::Ready;

	return true;
}

void StageController::ResetValues(int stageNumber)
{
	stageIndex_ = stageNumber - 1;
	score_ = 0;
	requiredScore_ = stages_[stageIndex_].requiredScore;
	remaining_ = stages_[stageIndex_].duration;
	stateTime_ = 0.0f;
	resetPending_ = true;
}

StageEvents StageController::Update(float deltaTime)
{
	StageEvents events;
	events.clearTargets = resetPending_;
	resetPending_ = false;

	if (deltaTime <= 0.0f || state_ == StageState::Idle) return events;

	//[윤지훈] 스테이지 업데이트 주기를 세팅함. 플레이중일 땐 1초, 그렇지 않으면 2초로
	// ex)숫자로 스테이지 시작 시, 2초 대기한 뒤 아래 switch문에서 Playing 상태로 넘어감.
	// 플레이 상태 제외 모든 상태 전환에서 2초 대기를 가짐
	// control.cpp와 같이 임계값을 먼저 확인하고, 미달일 때만 시간을 누적합니다.
	// 큰 지연을 임의로 따라잡아 여러 번 생성하지 않습니다.
	const float interval = state_ == StageState::Playing ? 1.0f : 2.0f;

	//[윤지훈] 스테이지 업데이트 주기보다 현재 경과시간이 적은 경우는 델타타임만 누적하고 리턴. 개선 필요
	if (stateTime_ < interval)
	{
		stateTime_ += deltaTime;

		return events;
	}

	//[윤지훈] 업데이트 주기가 되는 경우 경과시간을 0으로 리셋하는데, 여기서 시간 손실이 일어남. 개선 필요
	stateTime_ = 0.0f;

	switch (state_)
	{
	case StageState::Ready:
		state_ = StageState::Playing;
		break;
	case StageState::Playing:
		events.spawnCount = stages_[ruleIndex_].targetsPerSecond;
		//[윤지훈] 남은 시간에서 1만큼을 뺌. 개선 필요
		--remaining_; // 마지막 초에도 먼저 표적을 생성합니다.

		//[윤지훈] 남은 시간이 0인 경우 승패여부 판단
		if (remaining_ == 0)
		{
			if (score_ < requiredScore_)
			{
				//[윤지훈] 목표 도달 실패 시 1스테이지부터 다시 시작하도록 리셋
				ResetValues(1);
				state_ = StageState::GameOver;
			}
			else if (ruleIndex_ == 2)
			{
				//[윤지훈] 3스테이지인 경우 완료.
				// ruleIndex는 인덱스 값으로 바로 사용하기에 ResetValues에서 사용하는 인자값보다 1이 큼
				state_ = StageState::Finishing;
			}
			else
			{
				//[윤지훈] 아래 버그는 resetvalues에서 입력된 스테이지 값에 -1을 하여 인덱스로 사용해서 일어남.
				// 원본 stage2는 stage3 표시로 초기화한 후 stage2 콜백을 다시 예약합니다.
				// 이번 작업은 구조 리팩토링이므로 이 기존 버그를 별도로 고치지 않습니다.
				ResetValues(ruleIndex_ + 2);
				ruleIndex_ = 1;
				state_ = StageState::Ready;
			}
		}

		break;
	case StageState::GameOver:
		state_ = StageState::Idle;
		break;
	case StageState::Finishing:
		//[윤지훈] 스코어, 상태, 표적 리셋
		score_ = 0;
		state_ = StageState::Clear;
		resetPending_ = true;
		break;
	case StageState::Clear:
		//[윤지훈] 클리어 후 Idle로 전환
		state_ = StageState::Idle;
		break;
	default:
		break;
	}

	//[윤지훈] switch문을 돌면서 resetPending이 true가 될 수 있고,
	//resetPending 여부만 조사해서 갱신하면 위에서 true로 설정된 clearTargets가 꺼질 수 있으므로
	//OR문으로 검사함.
	events.clearTargets = events.clearTargets || resetPending_;
	resetPending_ = false;

	return events;
}
