#pragma once
#include "tools.h"
#include <map>
#include <vector>
#include <string>

// 거리 충돌 검사 대상 쌍
extern	std::map<	std::string,
		std::pair<	std::vector<Model*>,
					std::vector<Model*>>> collide_pair_range;

// 거리 충돌 검사
bool collision_range(Model& a, Model& b);
void add_collision_pair_range(const std::string& pair_name, Model* a, Model* b);
void handle_collisions();
void handle_collisions_range();