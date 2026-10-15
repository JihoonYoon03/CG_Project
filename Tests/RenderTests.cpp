#include "pch.h"
#include "RenderTests.h"
#include "Game/ShootingScene.h"

namespace
{
    void Check(bool passed, const char* message)
    {
        if (!passed)
        {
            throw std::runtime_error(std::string("Scene test failed: ") + message);
        }
    }

    void CheckOriginalScene()
    {
        ShootingScene scene;
        scene.Initialize();
        std::vector<Target*> targets;
        for (const auto& object : scene.GetObjects())
        {
            if (auto* target = dynamic_cast<Target*>(object.get()))
            {
                targets.push_back(target);
            }
            if (object->name == "Ground")
            {
                Check(object->transform.position == glm::vec3(0), "Original ground position");
                Check(object->GetWorldBox().min.y == 0 && object->GetWorldBox().max.y == 0, "Ground has no thickness");
                Check(object->GetMaterial()->color == glm::vec3(0.55f, 0.37f, 0.17f) &&
                          !object->GetMaterial()->useTexture && object->GetMaterial()->ambientOnly,
                      "Original brown ground rendering");
            }
            if (object->name == "Local player")
            {
                Check(object->visible, "Original player must be visible");
            }
        }
        Check(targets.size() == 90 && scene.GetObjects().size() == 99, "Original object counts");
        for (const auto* target : targets)
        {
            Check(!target->active, "Original targets start inactive");
            Check(target->GetMesh() == targets.front()->GetMesh(), "Targets share a mesh");
            Check(target->GetMaterial() == targets.front()->GetMaterial(), "Targets share a material");
            const auto p = target->transform.position;
            Check(p.x >= -5 && p.x <= 5 && p.y >= 0.5f && p.y <= 2 && p.z >= -5 && p.z <= -0.5f,
                  "Original target placement range");
        }
        Check(scene.GetLights().size() == 1 && scene.GetLights()[0].type == LightType::Point &&
                  scene.GetLights()[0].position == glm::vec3(100, 150, 110) &&
                  scene.GetLights()[0].color == glm::vec3(1) && scene.GetLights()[0].range == 0,
              "Original white light without attenuation");
        Input input;
        input.SetKey('f', true);
        scene.Update(0, input);
        Check(scene.GetLights()[0].enabled && scene.GetLights().size() == 1, "No flashlight control");
        input.Reset();
        targets[0]->transform.position = scene.GetCamera().position + scene.GetCamera().GetForward() * 4.0f;
        targets[1]->transform.position = scene.GetCamera().position + scene.GetCamera().GetForward() * 2.0f;
        targets[0]->active = targets[1]->active = true;
        PlayerCommand fire;
        fire.fire = true;
        scene.ApplyPlayerCommand(fire, 0);
        Check(targets[0]->active && targets[1]->active, "Original shots are resolved on world update");
        scene.Update(0, input);
        Check(!targets[0]->active && targets[1]->active, "Original target registration order");
        scene.Update(0, input);
        Check(!targets[1]->active && scene.GetHud().lines[1] == "point: 2", "Original persistent ray and idle scoring");
        targets[0]->active = true;
        scene.Update(0, input);
        Check(!targets[0]->active, "Last shot also affects reactivated targets");
        std::cout << "Scene checks passed: original layout, materials, controls, light, shot order and lifetime.\n";
    }

    class RenderTestScene : public ShootingScene
    {
    public:
        RenderTestScene(float yaw, float pitch, float recoilTime, float moveTime)
            : yaw_(yaw), pitch_(pitch), recoilTime_(recoilTime), moveTime_(moveTime)
        {
        }

        void Initialize() override
        {
            CheckOriginalScene();
            ShootingScene::Initialize();
            PlayerCommand snapshot;
            snapshot.lookDelta = {-yaw_ / 0.2f, -pitch_ / 0.2f};
            if (moveTime_ > 0)
            {
                snapshot.movement.y = -1;
            }
            ApplyPlayerCommand(snapshot, moveTime_);
            for (const auto& object : GetObjects())
            {
                if (auto* gun = dynamic_cast<Gun*>(object.get()))
                {
                    if (recoilTime_ >= 0)
                    {
                        gun->Fire();
                        gun->Update(recoilTime_ / 1.5f);
                        gun->Update(0);
                    }
                    gun->FollowCamera(camera_);
                }
            }
        }

        void Update(float, const Input&) override
        {
            // 테스트에서만 두 타입을 잠시 추가하여 공통 렌더링 경로를 검사합니다.
            // 마지막 캡처는 원래 장면의 초기 상태입니다.
            ++frame_;
            if (frame_ == 1)
            {
                lights_.push_back(Light::Directional({-1, -1, -1}, glm::vec3(1), 0.5f));
                lights_.push_back(Light::Spot(camera_.position, camera_.GetForward(), glm::vec3(1), 1, 10, 10, 20));
                Check(lights_[1].type == LightType::Directional && lights_[2].type == LightType::Spot,
                      "Light factories");
                debugColliders = true;
            }
            else if (frame_ == 2)
            {
                lights_.resize(1);
                debugColliders = false;
            }
        }

    private:
        int frame_ = 0;
        float yaw_, pitch_, recoilTime_, moveTime_;
    };
} // namespace

std::unique_ptr<Scene> CreateRenderTestScene(float yaw, float pitch, float recoilTime, float moveTime)
{
    return std::make_unique<RenderTestScene>(yaw, pitch, recoilTime, moveTime);
}
