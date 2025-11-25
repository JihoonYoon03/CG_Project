#pragma once
#include "tools.h"
#include <map>
#include <vector>
#include <string>

// 거리 충돌 검사 대상 쌍
extern	std::map<	std::string,
		std::pair<	std::vector<SphereCollider&>,
					std::vector<SphereCollider&>>> collide_pair_range;

// 거리 충돌 검사
bool collision_range(SphereCollider& a, SphereCollider& b);