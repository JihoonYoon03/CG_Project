#include "Background.h"

Objects::Objects(const std::string& filename, const glm::vec3& pos, const glm::vec3& size, const CollideMode& collider, const glm::vec3& defColor, const std::string& texture)
	: Model(filename, size, defColor, collider, texture) {
}