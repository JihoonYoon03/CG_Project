#pragma once
#include "pch.h"
#include "Scene.h"
#include "Player.h"
#include "Gun.h"
#include "Target.h"
#include "StageController.h"

class ShootingScene : public Scene
{
public:
	void Initialize() override;
	void Update(float deltaTime, const Input& input) override;
	Hud GetHud() const override;
	//[윤지훈] PlayerCommand에 입력을 통한 이동 정보가 저장되어있음
	void ApplyPlayerCommand(const PlayerCommand& command, float deltaTime);

private:
	//[윤지훈] 오브젝트 생성 및 씬에 추가하는 함수
	Object* AddProp(const std::string& name, const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material, const glm::vec3& position, const glm::vec3& scale, bool solid = false);
	void SpawnTarget();
	void Shoot();

	// 소유권은 Scene의 objects_에 있습니다. 아래 포인터는 현재 장면에서만 사용합니다.
	Player*				 player_ = nullptr;
	Gun*				 gun_ = nullptr;
	std::vector<Target*> targets_;
	StageController		 stages_;
	Ray					 shot_;
	//[윤지훈] 현재 hasShot은 단일 플레이어 특화 기능임.
	bool				 hasShot_ = false;
	//[윤지훈] 엔진 분리 필요
	std::mt19937		 random_{std::random_device{}()};
};
