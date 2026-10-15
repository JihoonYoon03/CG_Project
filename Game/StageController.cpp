#include "pch.h"
#include "StageController.h"

bool StageController::Start(int stageNumber)
{
    if (state_ == StageState::Ready || state_ == StageState::Playing)
    {
        return false;
    }
    if (stageNumber < 1 || stageNumber > static_cast<int>(stages_.size()))
    {
        return false;
    }
    stageIndex_ = stageNumber - 1;
    BeginStage();
    return true;
}

void StageController::BeginStage()
{
    state_ = StageState::Ready;
    stateTime_ = 0.0f;
    spawnTime_ = 0.0f;
    remaining_ = stages_[stageIndex_].duration;
    score_ = 0;
    resetPending_ = true;
}

StageEvents StageController::Update(float deltaTime)
{
    StageEvents events;
    events.clearTargets = resetPending_;
    resetPending_ = false;
    if (deltaTime <= 0.0f)
    {
        return events;
    }

    if (state_ == StageState::Ready)
    {
        stateTime_ += deltaTime;
        if (stateTime_ >= 2.0f)
        {
            state_ = StageState::Playing;
            stateTime_ = 0.0f;
        }
    }
    else if (state_ == StageState::Playing)
    {
        remaining_ = std::max(0.0f, remaining_ - deltaTime);
        spawnTime_ += deltaTime;
        if (remaining_ > 0.0f)
        {
            while (spawnTime_ >= 1.0f)
            {
                spawnTime_ -= 1.0f;
                events.spawnCount += stages_[stageIndex_].targetsPerSecond;
            }
        }
        else
        {
            events.clearTargets = true;
            if (score_ < stages_[stageIndex_].requiredScore)
            {
                state_ = StageState::GameOver;
            }
            else if (stageIndex_ + 1 == static_cast<int>(stages_.size()))
            {
                state_ = StageState::Clear;
            }
            else
            {
                ++stageIndex_;
                BeginStage();
                resetPending_ = false; // 이번 이벤트로 이미 초기화 요청을 전달합니다.
            }
        }
    }
    return events;
}

void StageController::AddScore()
{
    if (state_ == StageState::Playing)
    {
        ++score_;
    }
}
