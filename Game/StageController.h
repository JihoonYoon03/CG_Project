#pragma once
#include "pch.h"

struct StageDefinition
{
    float duration = 30.0f;
    int requiredScore = 20;
    int targetsPerSecond = 1;
};

enum class StageState
{
    Idle,
    Ready,
    Playing,
    GameOver,
    Clear
};

struct StageEvents
{
    bool clearTargets = false;
    int spawnCount = 0;
};

// 시간과 점수만 관리합니다. 렌더링, 입력, 오브젝트 생성은 Scene의 역할입니다.
class StageController
{
public:
    bool Start(int stageNumber);
    StageEvents Update(float deltaTime);
    void AddScore();

    StageState GetState() const
    {
        return state_;
    }

    int GetStageNumber() const
    {
        return stageIndex_ + 1;
    }

    int GetScore() const
    {
        return score_;
    }

    int GetRequiredScore() const
    {
        return stages_[stageIndex_].requiredScore;
    }

    int GetRemainingSeconds() const
    {
        return static_cast<int>(std::ceil(std::max(remaining_, 0.0f)));
    }

private:
    void BeginStage();
    // 새 스테이지의 규칙은 이 표에 추가합니다.
    const std::array<StageDefinition, 3> stages_{{{30.0f, 20, 1}, {30.0f, 40, 2}, {30.0f, 60, 3}}};
    StageState state_ = StageState::Idle;
    int stageIndex_ = 0;
    int score_ = 0;
    float remaining_ = 0.0f;
    float stateTime_ = 0.0f;
    float spawnTime_ = 0.0f;
    bool resetPending_ = false;
};
