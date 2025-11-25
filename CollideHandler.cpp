#include "CollideHandler.h"

// 거리 충돌 검사 대상 쌍
std::map<	std::string,
std::pair<	std::vector<SphereCollider&>,
			std::vector<SphereCollider&>>> collide_pair_range;