#pragma once
#include "tools.h"
#include <map>
#include <vector>
#include <string>
#include "Targets.h"
#include "Character.h"
#include "Background.h"

// 거리 충돌 검사 대상 쌍
extern	std::map<	std::string,
		std::pair<	std::vector<Model*>,
					std::vector<Model*>>> collide_pair_range;

// 광선 충돌 검사 대상 쌍
extern	std::map<	std::string,
		std::pair<	std::vector<Ray*>,
					std::vector<TargetDefault*>>> collide_pair_raycast;

// 거리 충돌 검사
bool collision_range(Model& a, Model& b);
bool collision_raycast(Ray& ray, TargetDefault& target);
void add_collision_pair_range(const std::string& pair_name, Model* a, Model* b);
void add_collision_pair_BB(const std::string& pair_name, Model* a, Model* b);
void add_collision_pair_raycast(const std::string& pair_name, Ray* ray, TargetDefault* target);
void delete_collision_pair_raycast(const std::string& pair_name, Ray* ray, TargetDefault* target);
void handle_collisions();
void handle_collisions_range();
void handle_collisions_BB();
void handle_collisions_raycast();