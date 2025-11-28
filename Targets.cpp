#include "Targets.h"

TargetDefault::TargetDefault(const std::string& filename, const glm::vec3& size, const CollideMode& collider, const glm::vec3& defColor)
	: Model(filename, size, defColor, collider) {

}