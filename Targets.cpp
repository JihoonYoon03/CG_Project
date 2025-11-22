#include "Targets.h"

TargetDefault::TargetDefault(const std::string& filename, const glm::vec3& size, const glm::vec3& defColor, const CollideMode& collider)
	: Model(filename, size, defColor, collider) {

}