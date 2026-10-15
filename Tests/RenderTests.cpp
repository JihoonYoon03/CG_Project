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

    class RenderTestScene : public ShootingScene
    {
    public:
        void Initialize() override
        {
            ShootingScene::Initialize();
            std::vector<Target*> targets;
            for (const auto& object : GetObjects())
            {
                if (auto* target = dynamic_cast<Target*>(object.get()))
                {
                    targets.push_back(target);
                }
            }
            Check(targets.size() == 90, "Target pool size");
            for (const auto* target : targets)
            {
                Check(target->GetMesh() == targets.front()->GetMesh(), "Targets must share one mesh");
                Check(target->GetMaterial() == targets.front()->GetMaterial(), "Targets must share one material");
            }
            Check(GetLights().size() == 3, "Scene must provide all three lights");
            Check(GetLights()[0].type == LightType::Directional && GetLights()[1].type == LightType::Point &&
                      GetLights()[2].type == LightType::Spot,
                  "Light types");

            Input start;
            start.SetKey('1', true);
            ShootingScene::Update(0, start);
            start.Reset();
            ShootingScene::Update(2, start);

            // 목록의 첫 표적이 더 멀어도 실제로 가까운 표적이 먼저 맞아야 합니다.
            const auto oldFar = targets[0]->transform.position;
            const auto oldNear = targets[1]->transform.position;
            targets[0]->transform.position = GetCamera().position + GetCamera().GetForward() * 4.0f;
            targets[1]->transform.position = GetCamera().position + GetCamera().GetForward() * 2.0f;
            targets[0]->active = targets[1]->active = true;
            PlayerCommand fire;
            fire.fire = true;
            ApplyPlayerCommand(fire, 0);
            Check(targets[0]->active && !targets[1]->active, "Nearest target must be hit first");
            ShootingScene::Update(0.1f, start);
            Check(targets[0]->active, "Old shot must not hit again on the next frame");
            ApplyPlayerCommand(fire, 0);
            Check(!targets[0]->active, "Second click must hit the remaining target");

            ShootingScene::Update(30, start);
            targets[0]->transform.position = oldFar;
            targets[1]->transform.position = oldNear;
            std::cout << "Scene checks passed: shared resources, 3 light types, nearest hit, single-shot lifetime.\n";
        }

        void Update(float deltaTime, const Input& input) override
        {
            ShootingScene::Update(deltaTime, input);
            elapsed_ += deltaTime;
            // 디버그 경로와 조명 개수 변경도 같은 검사에서 실행합니다.
            debugColliders = elapsed_ < 1.0f;
            lights_[2].enabled = elapsed_ >= 0.5f;
        }

    private:
        float elapsed_ = 0.0f;
    };
} // namespace

std::unique_ptr<Scene> CreateRenderTestScene()
{
    return std::make_unique<RenderTestScene>();
}
