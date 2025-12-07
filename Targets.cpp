#include "Targets.h"
#include "Variables.h"

TargetDefault::TargetDefault(const std::string& filename, const glm::vec3& size, const CollideMode& collider, const glm::vec3& defColor)
	: Model(filename, size, defColor, collider) {
}

void TargetDefault::HandleCollisionRange(const std::string& group, Model* other) {
	if (group == "target:target" ) {
		std::cout << "TargetDefault collided with another TargetDefault." << std::endl;
	}
}

void TargetDefault::HandleCollisionRaycast(const std::string& group, Ray* other) {
	if (group == "bullet:target") {
		if (hp <= 0) {
			hp = 0;
			enabled = false;
			point_count++;
		}
	}
}