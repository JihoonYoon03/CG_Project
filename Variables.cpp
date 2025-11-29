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