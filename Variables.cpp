#include "Variables.h"

Model* skydome;
Player* player;
Gun* gun = nullptr;
Camera* camera = nullptr;
std::vector<TargetDefault*> targets;
std::vector<Objects*> objects;

glm::vec3 lightColor{ 1.0f, 1.0f, 1.0f };
glm::vec3 lightPos{ 100.0f, 100.0f, -100.0f };

DebugCube* debugCube = nullptr;

int stage_level = 1; // 스테이지
int point_count = 0; // 점수
int target_point = 0; // 목표 점수
int timer_count = 0; // 남은 시간 타이머