#include "pch.h"
#include "ShootingScene.h"

Object* ShootingScene::AddProp(const std::string& name, const std::shared_ptr<Mesh>& mesh,
                               const std::shared_ptr<Material>& material, const glm::vec3& position,
                               const glm::vec3& scale, bool solid)
{
    auto object = std::make_unique<Object>(mesh, material);
    object->name = name;
    object->transform.position = position;
    object->transform.scale = scale;
    object->blocksMovement = solid;
    object->collider = solid ? ColliderType::Box : ColliderType::None;
    return AddObject(std::move(object));
}

void ShootingScene::Initialize()
{
    const auto shader = std::make_shared<Shader>("Shaders/Lit.vert", "Shaders/Lit.frag");
    const auto cube = assets_.LoadMesh("models/Cube.obj");
    const auto sphere = assets_.LoadMesh("models/Sphere.obj");
    auto sky = std::make_shared<Material>(shader);
    sky->texture = assets_.LoadTexture("models/SkydomeTex.png");
    sky->unlit = true;
    sky->doubleSided = true;
    auto* dome = AddProp("Sky", sphere, sky, glm::vec3(0), glm::vec3(150));
    dome->transform.rotation.y = -90.0f;

    auto ground = std::make_shared<Material>(shader);
    ground->texture = assets_.LoadTexture("models/GroundTex.png");
    AddProp("Ground", cube, ground, {0, -0.05f, 0}, {5, 0.05f, 5});
    auto fence = std::make_shared<Material>(shader);
    fence->color = glm::vec3(0.9f);
    AddProp("Shooting line", cube, fence, {0, 0.375f, -0.25f}, {5, 0.75f, 0.05f}, true);
    AddProp("Right fence", cube, fence, {5, 0.375f, 0}, {0.05f, 0.75f, 5}, true);
    AddProp("Left fence", cube, fence, {-5, 0.375f, 0}, {0.05f, 0.75f, 5}, true);
    AddProp("Back fence", cube, fence, {0, 0.375f, 5}, {5, 0.75f, 0.05f}, true);
    AddProp("Front fence", cube, fence, {0, 0.375f, -5}, {5, 0.75f, 0.05f}, true);

    auto playerMaterial = std::make_shared<Material>(shader);
    playerMaterial->color = {0.8f, 0.8f, 0.2f};
    auto player = std::make_unique<Player>(cube, playerMaterial);
    player_ = player.get();
    player->name = "Local player";
    player->transform.position = {0, 1, 1};
    player->transform.scale = glm::vec3(0.25f);
    player->collider = ColliderType::Box;
    player->visible = false;
    AddObject(std::move(player));
    player_->UpdateCamera(camera_);

    auto gunMaterial = std::make_shared<Material>(shader);
    gunMaterial->color = glm::vec3(0.2f);
    auto gun = std::make_unique<Gun>(assets_.LoadMesh("models/Pistol.obj"), gunMaterial);
    gun_ = gun.get();
    gun->name = "Pistol";
    gun->transform.scale = glm::vec3(0.025f);
    gun->FollowCamera(camera_);
    AddObject(std::move(gun));

    auto targetMaterial = std::make_shared<Material>(shader);
    targetMaterial->color = {1.0f, 0.1f, 0.1f};
    std::uniform_real_distribution<float> x(-4.7f, 4.7f), y(0.5f, 2.0f), z(-4.7f, -0.6f);
    for (int i = 0; i < 90; ++i)
    {
        auto target = std::make_unique<Target>(sphere, targetMaterial);
        target->name = "Target " + std::to_string(i + 1);
        target->transform.position = {x(random_), y(random_), z(random_)};
        target->transform.scale = glm::vec3(0.15f);
        target->collider = ColliderType::Sphere;
        target->active = false;
        targets_.push_back(target.get());
        AddObject(std::move(target));
    }

    lights_.push_back(Light::Directional({-0.5f, -1.0f, -0.3f}, {1.0f, 0.96f, 0.9f}, 0.8f));
    lights_.push_back(Light::Point({-3, 2, -2}, {0.4f, 0.65f, 1.0f}, 0.7f, 6.0f));
    lights_.push_back(Light::Spot(camera_.position, camera_.GetForward(), glm::vec3(1), 0.8f, 12.0f, 12.0f, 22.0f));
}

void ShootingScene::Update(float deltaTime, const Input& input)
{
    for (unsigned char key = '1'; key <= '3'; ++key)
    {
        if (input.WasPressed(key))
        {
            stages_.Start(key - '0');
        }
    }
    if (input.WasPressed('/'))
    {
        debugColliders = !debugColliders;
    }

    const StageEvents events = stages_.Update(deltaTime);
    if (events.clearTargets)
    {
        for (auto* target : targets_)
        {
            target->active = false;
        }
    }
    for (int i = 0; i < events.spawnCount; ++i)
    {
        SpawnTarget();
    }

    PlayerCommand command;
    command.movement.x = static_cast<float>(input.IsDown('d')) - static_cast<float>(input.IsDown('a'));
    command.movement.y = static_cast<float>(input.IsDown('s')) - static_cast<float>(input.IsDown('w'));
    command.lookDelta = input.mouseDelta;
    command.fire = input.firePressed;
    ApplyPlayerCommand(command, deltaTime);
    Scene::Update(deltaTime, input);
    gun_->FollowCamera(camera_);
    // 장면이 소유한 손전등을 카메라에 맞춥니다. F 키로 켜고 끌 수 있습니다.
    if (input.WasPressed('f'))
    {
        lights_[2].enabled = !lights_[2].enabled;
    }
    lights_[2].position = camera_.position;
    lights_[2].direction = camera_.GetForward();
}

void ShootingScene::ApplyPlayerCommand(const PlayerCommand& command, float deltaTime)
{
    player_->ApplyCommand(command, deltaTime, *this);
    player_->UpdateCamera(camera_);
    if (command.fire)
    {
        gun_->Fire();
        if (stages_.GetState() == StageState::Playing)
        {
            Shoot();
        }
    }
}

void ShootingScene::SpawnTarget()
{
    std::vector<Target*> available;
    for (auto* target : targets_)
    {
        if (!target->active)
        {
            available.push_back(target);
        }
    }
    if (available.empty())
    {
        return;
    }
    std::uniform_int_distribution<size_t> pick(0, available.size() - 1);
    available[pick(random_)]->active = true;
}

void ShootingScene::Shoot()
{
    const Ray ray{camera_.position, camera_.GetForward()};
    Target* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    // 광선을 저장하지 않습니다. 한 번의 발사는 가장 가까운 표적 하나만 처리합니다.
    for (auto* target : targets_)
    {
        float distance = 0.0f;
        if (target->active && Collision::Raycast(ray, target->GetWorldSphere(), distance) && distance < nearestDistance)
        {
            nearest = target;
            nearestDistance = distance;
        }
    }
    if (nearest && nearest->Hit())
    {
        stages_.AddScore();
    }
}

Hud ShootingScene::GetHud() const
{
    Hud hud;
    hud.lines = {
        "Stage: " + std::to_string(stages_.GetStageNumber()) + "   Score: " + std::to_string(stages_.GetScore()) +
            " / " + std::to_string(stages_.GetRequiredScore()) +
            "   Time: " + std::to_string(stages_.GetRemainingSeconds()),
        "WASD: move | Mouse: aim | Click: shoot | 1-3: start | F: flashlight | /: colliders | Esc: cursor | Q: quit"};
    switch (stages_.GetState())
    {
        case StageState::Idle:
            hud.message = "Press 1, 2 or 3 to start";
            break;
        case StageState::Ready:
            hud.message = "Stage " + std::to_string(stages_.GetStageNumber());
            break;
        case StageState::GameOver:
            hud.message = "Game Over - Press 1, 2 or 3";
            break;
        case StageState::Clear:
            hud.message = "Game Clear! - Press 1 to restart";
            break;
        default:
            break;
    }
    return hud;
}
