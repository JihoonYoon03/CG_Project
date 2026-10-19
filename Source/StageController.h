#pragma once
#include "pch.h"

//[윤지훈] 현재 에임랩 스테이지의 남은시간/목표점수/초당 타깃 생성
// 아래 기본값은 1스테이지 기준
struct StageDefinition
{
	int duration = 30;
	int requiredScore = 20;
	int targetsPerSecond = 1;
};

//[윤지훈] 스테이지 현재 상태를 나타냄
enum class StageState
{
	Idle,
	Ready,
	Playing,
	GameOver,
	Finishing,
	Clear
};

//[윤지훈] 현재 ShootingScene이 처리해야 할 이벤트를 나타냄.
//clearTargets = 표적 전체 리셋, spawnCount = 남은 표적 스폰 수
struct StageEvents
{
	bool clearTargets = false;
	int	 spawnCount = 0;
};

// 렌더링과 표적 생성은 Scene에 맡기고 원본의 시간/점수 전이만 관리합니다.
class StageController
{
public:
	bool Start(int stageNumber);
	//[윤지훈] 반환값은 ShootingScene에서 받아서 표적 생성 or 전체 리셋 여부를 결정함
	StageEvents Update(float deltaTime);

	void AddScore() { ++score_; }
	StageState GetState() const { return state_; }
	int GetStageNumber() const { return stageIndex_ + 1; }
	int GetScore() const { return score_; }
	int GetRequiredScore() const { return requiredScore_; }
	int GetRemainingSeconds() const { return remaining_; }

private:
	void ResetValues(int stageNumber);

	const std::array<StageDefinition, 3> stages_{{{30, 20, 1}, {30, 40, 2}, {30, 60, 3}}};
	StageState							 state_ = StageState::Idle;
	int									 stageIndex_ = 0;
	int									 ruleIndex_ = 0;
	int									 score_ = 0;
	int									 requiredScore_ = 0;
	int									 remaining_ = 0;
	float								 stateTime_ = 0.0f;
	bool								 resetPending_ = false;
};
