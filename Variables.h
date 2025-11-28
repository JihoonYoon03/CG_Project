#pragma once
#include "Character.h"
#include "Targets.h"
#include "Background.h"

extern Player* player;
extern Gun* gun;
extern Camera* camera;
extern std::vector<TargetDefault*> targets;
extern std::vector<Objects*> objects;

extern glm::vec3 lightColor;
extern glm::vec3 lightPos;
extern glm::vec3 viewPos;
extern float shininess;