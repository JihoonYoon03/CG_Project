#pragma once
#include "basic.h"

class Ray;

class TargetDefault : public Model {
protected:
	float hp = 0;
	
public:
	TargetDefault(const std::string& filename, const glm::vec3& size = { 1.0f, 1.0f, 1.0f }, const CollideMode& collider = NONE, const glm::vec3& defColor = { 1.0f, 0.1f, 0.1f });
	float return_hp() { return hp; }

	void HandleCollisionRange(const std::string& group, Model* other) override;
	void HandleCollisionRaycast(const std::string& group, Ray* other);
};