#include "CollideHandler.h"

// 거리 충돌 검사 대상 쌍
std::map<	std::string,
std::pair<	std::vector<Model*>,
			std::vector<Model*>>> collide_pair_range;


// 거리 충돌 검사
bool collision_range(Model& a, Model& b) {
	return true;
}

// 충돌 검사 대상 추가
void add_collision_pair_range(const std::string& pair_name, Model* a, Model* b) {
	if (a != nullptr)
		collide_pair_range[pair_name].first.push_back(a);
	if (b != nullptr)
		collide_pair_range[pair_name].second.push_back(b);
}

void handle_collisions_range() {
	// 충돌 그룹에 대하여
	for (auto& pair : collide_pair_range) {

		// 해당 그룹의 객체 리스트에 대하여
		auto& groupA = pair.second.first;
		auto& groupB = pair.second.second;

		// 리스트의 각 객체들 간 충돌 검사
		for (auto& objA : groupA) {
			for (auto& objB : groupB) {
				if (objA != nullptr && objB != nullptr) {
					if (collision_range(*objA, *objB)) {
						// 충돌 처리 로직
					}
				}
			}
		}
	}
}