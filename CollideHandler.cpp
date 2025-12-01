#include "CollideHandler.h"

// 거리 충돌 검사 대상 쌍
std::map<	std::string,
	std::pair<	std::vector<Model*>,
	std::vector<Model*>>> collide_pair_range;

std::map<	std::string,
	std::pair<	std::vector<Model*>,
	std::vector<Model*>>> collide_pair_BB;

// 광선 충돌 검사 대상 쌍
std::map<	std::string,
	std::pair<	std::vector<Ray*>,
	std::vector<TargetDefault*>>> collide_pair_raycast;

// 거리 충돌 검사
bool collision_range(Model& a, Model& b) {
	GLfloat distance = glm::length(a.retDistTo() - b.retDistTo()); // 월드 좌표 기준 거리 계산
	GLfloat radius_a = a.getSphereCollider()->getRadius();
	GLfloat radius_b = b.getSphereCollider()->getRadius();
	if (distance <= (radius_a + radius_b)) {
		return true;
	}
}

// 바운딩 박스 충돌 검사
bool collision_BB(Model& a, Model& b) {
	// 바운딩 박스 매커니즘 생성 필요
	GLfloat distance = glm::length(a.retDistTo() - b.retDistTo()); // 월드 좌표 기준 거리 계산
	GLfloat radius_a = a.getSphereCollider()->getRadius();
	GLfloat radius_b = b.getSphereCollider()->getRadius();
	if (distance <= (radius_a + radius_b)) {
		return true;
	}
}

// 광선 충돌 검사
bool collision_raycast(Ray& ray, TargetDefault& target) {
	glm::vec3 ray_origin = ray.getOrigin();
	glm::vec3 ray_direction = ray.getDirection();
	glm::vec3 target_center = target.retDistTo(); // 월드 좌표
	GLfloat radius = target.getSphereCollider()->getRadius();

	// 광선-구 충돌 검사 공식 적용
	glm::vec3 oc = ray_origin - target_center;			// 구 중심에서 광선 원점으로 향하는 벡터
	GLfloat a = glm::dot(ray_direction, ray_direction);	// 방향벡터의 길이 제곱
	GLfloat b = 2.0f * glm::dot(oc, ray_direction);		// 구 중심에서 광선 원점으로 향하는 벡터와 방향벡터의 내적
	GLfloat c = glm::dot(oc, oc) - radius * radius;		// 구 중심에서 광선 원점으로 향하는 벡터의 길이 제곱에서 구 반지름 제곱을 뺀 값

	GLfloat discriminant = b * b - 4 * a * c;		// 판별식 계산
	if (discriminant <= 0)
		return false; // 교차 X

	GLfloat sqrtD = std::sqrt(discriminant);
	GLfloat t1 = (-b - sqrtD) / (2.0f * a); // 더 가까운 충돌점
	GLfloat t2 = (-b + sqrtD) / (2.0f * a);

	GLfloat t = t1;
	if (t < 0.0f) t = t2;		// t1이 시점 뒤쪽이므로 t2도 검사
	if (t < 0.0f) return false; // 둘 다 시점 뒤쪽이므로 충돌 X

	return true;
}

// 충돌 검사 대상 추가
void add_collision_pair_range(const std::string& pair_name, Model* a, Model* b) {
	if (a != nullptr)
		collide_pair_range[pair_name].first.push_back(a);
	if (b != nullptr)
		collide_pair_range[pair_name].second.push_back(b);
}

void add_collision_pair_BB(const std::string& pair_name, Model* a, Model* b) {
	if (a != nullptr)
		collide_pair_BB[pair_name].first.push_back(a);
	if (b != nullptr)
		collide_pair_BB[pair_name].second.push_back(b);
}

void add_collision_pair_raycast(const std::string& pair_name, Ray* ray, TargetDefault* target) {
	if (ray != nullptr)
		collide_pair_raycast[pair_name].first.push_back(ray);
	if (target != nullptr)
		collide_pair_raycast[pair_name].second.push_back(target);
}

void delete_collision_pair_raycast(const std::string& pair_name, Ray* ray, TargetDefault* target) {
	// 해당 그룹 쌍 각각의 시작부터 끝까지 탐색하며 일치하는 객체 삭제
	if (ray != nullptr) {
		for (auto it = collide_pair_raycast[pair_name].first.begin(); it != collide_pair_raycast[pair_name].first.end(); ) {
			if (*it == ray) {
				it = collide_pair_raycast[pair_name].first.erase(it);
			}
			else {
				++it;
			}
		}
	}
	if (target != nullptr) {
		for (auto it = collide_pair_raycast[pair_name].second.begin(); it != collide_pair_raycast[pair_name].second.end(); ) {
			if (*it == target) {
				it = collide_pair_raycast[pair_name].second.erase(it);
			}
			else {
				++it;
			}
		}
	}
}

void handle_collisions() {
	handle_collisions_range();
	handle_collisions_BB();
	handle_collisions_raycast();
}

void handle_collisions_range() {
	// 충돌 그룹에 대하여
	for (auto& pair : collide_pair_range) {

		// 해당 그룹의 객체 리스트에 대하여
		auto& group = pair.first;
		auto& groupA = pair.second.first;
		auto& groupB = pair.second.second;

		// 리스트의 각 객체들 간 충돌 검사
		for (auto& objA : groupA) {
			for (auto& objB : groupB) {
				if (objA != nullptr && objB != nullptr) {
					if (objA == objB) continue; // 자기 자신과의 충돌 검사 제외. 포인터 주소 비교
					if (collision_range(*objA, *objB)) {
						// 충돌 처리 로직
						objA->HandleCollisionRange(group, objB);
						objB->HandleCollisionRange(group, objA);
					}
				}
			}
		}
	}
}

void handle_collisions_BB() {
	// 충돌 그룹에 대하여
	for (auto& pair : collide_pair_range) {

		// 해당 그룹의 객체 리스트에 대하여
		auto& group = pair.first;
		auto& groupA = pair.second.first;
		auto& groupB = pair.second.second;

		// 리스트의 각 객체들 간 충돌 검사
		for (auto& objA : groupA) {
			for (auto& objB : groupB) {
				if (objA != nullptr && objB != nullptr) {
					if (objA == objB) continue; // 자기 자신과의 충돌 검사 제외. 포인터 주소 비교
					if (collision_BB(*objA, *objB)) {
						// 충돌 처리 로직
						objA->HandleCollisionBB(group, objB);
						objB->HandleCollisionBB(group, objA);
					}
				}
			}
		}
	}
}

void handle_collisions_raycast() {
	// 충돌 그룹에 대하여
	for (auto& pair : collide_pair_raycast) {

		// 해당 그룹의 객체 리스트에 대하여
		auto& group = pair.first;
		auto& groupRay = pair.second.first;
		auto& groupTarget = pair.second.second;

		// 리스트의 각 객체들 간 충돌 검사
		for (auto& ray : groupRay) {
			int penetration = ray->getPenetrationCount();
			for (auto& target : groupTarget) {
				if (ray != nullptr && target != nullptr) {
					if (collision_raycast(*ray, *target)) {
						penetration--;
						ray->HandleCollisionRaycast<TargetDefault>(group, target);
						target->HandleCollisionRaycast(group, ray);
						delete_collision_pair_raycast(group, nullptr, target);
						if (penetration <= 0) break; // 더 이상 검사하지 않음
					}
				}
			}
		}
	}
}