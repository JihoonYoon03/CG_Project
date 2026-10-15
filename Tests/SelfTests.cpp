#include "pch.h"
#include "SelfTests.h"
#include "Engine/Collision.h"
#include "Engine/Input.h"
#include "Engine/Scene.h"
#include "Game/StageController.h"
#include "Game/Player.h"
#include "Game/Target.h"
#include "Game/Gun.h"

namespace
{
    int checks = 0;

    void Check(bool condition, const char* message)
    {
        ++checks;
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    bool Near(float a, float b)
    {
        return std::abs(a - b) < 0.001f;
    }

    class TestScene : public Scene
    {
    public:
        void Initialize() override {}
    };

    void TestCollision()
    {
        float distance = -1;
        const Ray ray{{0, 0, 0}, {0, 0, -2}};
        Check(Collision::Raycast(ray, Sphere{{0, 0, -5}, 1}, distance) && Near(distance, 4),
              "Sphere hit distance must use world units");
        Check(!Collision::Raycast(ray, Sphere{{0, 0, 5}, 1}, distance), "A ray must not hit behind its origin");
        Check(!Collision::Raycast(ray, Sphere{{1, 0, -5}, 1}, distance), "Original tangent rule must be preserved");
        Check(Collision::Raycast(ray, Sphere{{0, 0, 0}, 1}, distance) && Near(distance, 1),
              "Original ray must use the exit intersection from inside a sphere");
        Check(!Collision::Raycast(Ray{{0, 0, 0}, {0, 0, 0}}, Sphere{{0, 0, -5}, 1}, distance),
              "Zero direction is invalid");
        const Box box{{-1, -1, -6}, {1, 1, -4}};
        Check(Collision::Raycast(ray, box, distance) && Near(distance, 4), "Box slab intersection failed");
        Check(!Collision::Raycast(Ray{{2, 0, 0}, {0, 0, -1}}, box, distance), "Parallel ray outside slab must miss");
        Check(!Collision::Overlaps(Box{{0, 0, 0}, {1, 1, 1}}, Box{{0, 2, 0}, {1, 3, 1}}),
              "Box overlap must include Y axis");
        Check(!Collision::Overlaps(Box{{0, 0, 0}, {1, 1, 1}}, Box{{1, 0, 0}, {2, 1, 1}}),
              "Touching boxes must allow movement along a wall");
        Check(Collision::Overlaps(Box{{0, 0, 0}, {1, 1, 1}}, Box{{0.5f, 0, 0}, {2, 1, 1}}),
              "Overlapping boxes were missed");
    }

    void StepTimer(StageController& stages, float seconds)
    {
        stages.Update(seconds);
        stages.Update(0.001f);
    }

    void TestStages()
    {
        StageController stages;
        Check(stages.GetRequiredScore() == 0 && stages.GetRemainingSeconds() == 0, "Original idle HUD values");
        stages.AddScore();
        Check(stages.GetScore() == 1, "Original collision callbacks score regardless of stage state");
        Check(!stages.Start(0) && !stages.Start(4), "Invalid stage selection");
        Check(stages.Start(1) && !stages.Start(2), "Running stage cannot be restarted");
        Check(stages.Update(2).clearTargets, "Starting a stage clears targets");
        stages.Update(0.001f);
        Check(stages.GetState() == StageState::Playing, "Two-second ready message");
        for (int i = 0; i < 20; ++i)
        {
            stages.AddScore();
        }
        int spawns = 0;
        for (int second = 0; second < 30; ++second)
        {
            stages.Update(1);
            spawns += stages.Update(0.001f).spawnCount;
        }
        Check(spawns == 30 && stages.GetState() == StageState::Ready && stages.GetStageNumber() == 2,
              "Stage 1 includes its final spawn before resetting");
        StepTimer(stages, 2);
        for (int i = 0; i < 40; ++i)
        {
            stages.AddScore();
        }
        for (int second = 0; second < 30; ++second)
        {
            StepTimer(stages, 1);
        }
        Check(stages.GetStageNumber() == 3 && stages.GetRequiredScore() == 60, "Original stage 3 display");
        StepTimer(stages, 2);
        stages.Update(1);
        Check(stages.Update(0.001f).spawnCount == 2, "Original stage2 callback quirk is preserved");

        StageController failed;
        failed.Start(2);
        StepTimer(failed, 2);
        for (int second = 0; second < 30; ++second)
        {
            StepTimer(failed, 1);
        }
        Check(failed.GetState() == StageState::GameOver && failed.GetStageNumber() == 1 && failed.GetScore() == 0 &&
                  failed.GetRemainingSeconds() == 30 && failed.GetRequiredScore() == 20,
              "Failure immediately resets the original HUD values");
        Check(!failed.Start(1), "Game Over blocks start during its two-second message");
        StepTimer(failed, 2);
        Check(failed.GetState() == StageState::Idle && failed.Start(1), "Game Over expires after two seconds");

        StageController clear;
        clear.Start(3);
        StepTimer(clear, 2);
        for (int i = 0; i < 60; ++i)
        {
            clear.AddScore();
        }
        for (int second = 0; second < 30; ++second)
        {
            StepTimer(clear, 1);
        }
        Check(clear.GetState() == StageState::Finishing && clear.GetScore() == 60, "Original clear delay");
        clear.Update(2);
        Check(clear.Update(0.001f).clearTargets && clear.GetState() == StageState::Clear && clear.GetScore() == 0,
              "Clear message resets score and targets after the delay");
        Check(clear.Start(2), "Original clear message allows stage selection");
        StageController expire;
        expire.Start(3);
        StepTimer(expire, 2);
        for (int i = 0; i < 60; ++i)
        {
            expire.AddScore();
        }
        for (int second = 0; second < 30; ++second)
        {
            StepTimer(expire, 1);
        }
        StepTimer(expire, 2);
        StepTimer(expire, 2);
        Check(expire.GetState() == StageState::Idle, "Clear message expires");
    }

    bool MatrixNear(const glm::mat4& a, const glm::mat4& b)
    {
        for (int column = 0; column < 4; ++column)
        {
            for (int row = 0; row < 4; ++row)
            {
                if (!Near(a[column][row], b[column][row]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    // 28561f6 Character.cpp + tools.cpp + draw.h의 원본 식을 독립적으로 구성합니다.
    glm::mat4 OriginalGunMatrix(const Camera& camera, float t)
    {
        float amount = t < 0.05f ? t / 0.05f : (1.0f - t) / 0.95f;
        const auto pitch = glm::angleAxis(glm::radians(camera.pitch), glm::vec3(1, 0, 0));
        const auto yaw = glm::angleAxis(glm::radians(camera.yaw), glm::vec3(0, 1, 0));
        const auto cameraRotation = glm::translate(glm::mat4(1), camera.position) * glm::mat4_cast(yaw * pitch) *
                                    glm::translate(glm::mat4(1), -camera.position);
        const auto parent = glm::translate(glm::mat4(1), camera.position - glm::vec3(0, 0.75f, -0.4f));
        const auto scale = glm::scale(glm::mat4(1), glm::vec3(0.025f));
        const auto base = glm::translate(glm::mat4(1), glm::vec3(0.2f, 0.35f, -0.8f)) *
                          glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(0, 1, 0)) * scale;
        const auto recoil = glm::angleAxis(glm::radians(-15.0f * amount), glm::vec3(1, 0, 0));
        const glm::vec3 position = glm::inverse(scale) * glm::vec4(0, 0.1f * amount, -0.2f * amount, 0);
        return cameraRotation * parent * base * glm::translate(glm::mat4_cast(recoil), position);
    }

    void TestGun()
    {
        for (float yaw : {0.0f, 90.0f, 180.0f, 270.0f})
        {
            for (float pitch : {-89.0f, -45.0f, 0.0f, 45.0f, 89.0f})
            {
                for (float time : {0.0f, 0.025f, 0.05f, 0.4f, 0.9f, 1.0f})
                {
                    Camera camera;
                    camera.yaw = yaw;
                    camera.pitch = pitch;
                    camera.position = {2.5f, 1.75f, -1.0f};
                    Gun gun;
                    gun.transform.scale = glm::vec3(0.025f);
                    gun.Fire();
                    gun.Update(time / 1.5f);
                    gun.Update(0);
                    gun.FollowCamera(camera);
                    Check(MatrixNear(gun.GetModelMatrix(), OriginalGunMatrix(camera, time)),
                          "Gun matrix differs from original");
                    camera.position += glm::vec3(0.3f, 0, -0.5f);
                    gun.FollowCamera(camera);
                    Check(MatrixNear(gun.GetModelMatrix(), OriginalGunMatrix(camera, time)),
                          "Moving gun differs from original");
                    gun.Fire();
                    gun.Update(0);
                    gun.FollowCamera(camera);
                    Check(MatrixNear(gun.GetModelMatrix(), OriginalGunMatrix(camera, 0)),
                          "Repeated click must restart recoil");
                }
            }
        }
        Gun gun;
        gun.transform.scale = glm::vec3(0.025f);
        Camera camera;
        gun.Fire();
        float t = 0;
        for (int frame = 0; frame < 170; ++frame)
        {
            const float dt = frame % 2 ? 0.006f : 0.004f;
            gun.Update(dt);
            gun.FollowCamera(camera);
            Check(MatrixNear(gun.GetModelMatrix(), OriginalGunMatrix(camera, t)),
                  "Recoil time curve differs from original");
            t = std::min(1.0f, t + 1.5f * dt);
        }
        Check(Near(camera.nearPlane, 0.1f) && Near(camera.farPlane, 200), "Original projection clipping");
    }

    void TestObjectsAndInput()
    {
        TestScene scene;
        auto object = std::make_unique<Object>();
        object->transform.scale = {2, 4, 6};
        object->transform.rotation.y = 90;
        Object* pointer = scene.AddObject(std::move(object));
        const Box bounds = pointer->GetWorldBox();
        Check(Near(bounds.max.x, 3) && Near(bounds.max.z, 1), "Rotated box bounds must swap X/Z extents");
        const uint32_t id = pointer->GetId();
        Check(scene.FindObject(id) == pointer, "Scene object lookup failed");
        Check(scene.RemoveObject(id) && !scene.FindObject(id), "Scene removal failed");
        Object* next = scene.AddObject(std::make_unique<Object>());
        Check(next->GetId() != id, "Removed object IDs must not be reused");

        Input input;
        input.SetKey('W', true);
        Check(!input.IsDown('w') && input.IsDown('W'), "Original controls are case sensitive");
        input.SetKey('w', true);
        input.EndFrame();
        Check(input.IsDown('w') && !input.WasPressed('w'), "Held keys survive frame boundaries");
        input.SetKey('w', true);
        Check(input.WasPressed('w'), "Original repeated keyboard callbacks are accepted");
        input.Reset();
        Check(!input.IsDown('w'), "Reset clears held keys");

        Target target;
        Check(target.Hit() && !target.Hit(), "A disabled target must not score twice");
    }

    void TestMovement()
    {
        TestScene scene;
        auto wall = std::make_unique<Object>();
        wall->transform.position = {1, 0, 0};
        wall->collider = ColliderType::Box;
        wall->blocksMovement = true;
        scene.AddObject(std::move(wall));
        Player player;
        PlayerCommand command;
        command.movement = {1, 0};
        player.ApplyCommand(command, 0.01f, scene);
        Check(Near(player.transform.position.x, 0), "Player must not enter a wall");
        command.movement = {1, -1};
        player.ApplyCommand(command, 0.01f, scene);
        Check(Near(player.transform.position.x, 0) && Near(player.transform.position.z, 0),
              "Original collision cancels the whole diagonal movement");
        Camera camera;
        player.UpdateCamera(camera);
        Check(camera.position.x > player.transform.position.x, "Original camera sample precedes collision rollback");
        const auto sampledEye = camera.position;
        PlayerCommand stop;
        player.ApplyCommand(stop, 0.01f, scene);
        player.UpdateCamera(camera);
        Check(camera.position == sampledEye, "Original stationary eye sample is retained");
        command.movement = {0, 0};
        command.lookDelta = {0, 10000};
        player.ApplyCommand(command, 0.01f, scene);
        Check(Near(player.pitch, -89), "Camera pitch must be clamped");
    }
} // namespace

int RunSelfTests()
{
    try
    {
        TestCollision();
        TestStages();
        TestObjectsAndInput();
        TestMovement();
        TestGun();
        std::cout << "All " << checks << " logic checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
