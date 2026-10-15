#include "pch.h"
#include "SelfTests.h"
#include "Engine/Collision.h"
#include "Engine/Input.h"
#include "Engine/Scene.h"
#include "Game/StageController.h"
#include "Game/Player.h"
#include "Game/Target.h"

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
        Check(Collision::Raycast(ray, Sphere{{1, 0, -5}, 1}, distance) && Near(distance, 5), "Tangent ray must hit");
        Check(Collision::Raycast(ray, Sphere{{0, 0, 0}, 1}, distance) && Near(distance, 0),
              "Origin inside sphere must hit immediately");
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

    void TestStages()
    {
        StageController stages;
        stages.AddScore();
        Check(stages.GetScore() == 0, "Idle hits must not score");
        Check(!stages.Start(0) && !stages.Start(4), "Invalid stage selection must fail");
        Check(stages.Start(1), "Stage 1 must start");
        Check(!stages.Start(2), "Cannot restart a running stage");
        Check(stages.Update(2).clearTargets && stages.GetState() == StageState::Playing,
              "Ready state transition failed");
        Check(stages.Update(1).spawnCount == 1, "Stage 1 spawn rate failed");
        for (int stage = 1; stage <= 3; ++stage)
        {
            if (stage > 1)
            {
                stages.Update(2);
                Check(stages.Update(1).spawnCount == stage, "Next stage must use its own spawn rate");
            }
            for (int score = 0; score < stage * 20; ++score)
            {
                stages.AddScore();
            }
            Check(stages.GetRequiredScore() == stage * 20, "Stage score threshold failed");
            Check(stages.Update(29).clearTargets, "End of stage must clear targets");
            if (stage < 3)
            {
                Check(stages.GetStageNumber() == stage + 1 && stages.GetState() == StageState::Ready,
                      "Stage progression failed");
            }
        }
        Check(stages.GetState() == StageState::Clear, "Third stage must complete the game");
        Check(stages.Start(2), "Restart after clear must work");
        stages.Update(2);
        stages.Update(30);
        Check(stages.GetState() == StageState::GameOver, "Insufficient score must end the game");
        Check(stages.Start(1) && stages.GetScore() == 0, "Restart after failure must reset score");
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
        Check(input.IsDown('w') && input.WasPressed('w'), "Input normalization failed");
        input.EndFrame();
        Check(input.IsDown('w') && !input.WasPressed('w'), "Held keys must survive frame boundaries");
        input.SetKey('w', true);
        Check(!input.WasPressed('w'), "Repeated key events must not trigger a new press");
        input.Reset();
        Check(!input.IsDown('w'), "Lost capture must clear held keys");

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
        Check(Near(player.transform.position.x, 0) && player.transform.position.z < 0,
              "Player must slide along a wall");
        Camera camera;
        player.UpdateCamera(camera);
        Check(Near(camera.position.x, player.transform.position.x), "Camera must follow resolved player position");
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
        std::cout << "All " << checks << " logic checks passed.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
