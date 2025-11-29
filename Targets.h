#pragma once
#include "basic.h"

class Ray;

class TargetDefault : public Model {
protected:
	float hp = 0;
	
public:
	TargetDefault(const std::string& filename, const glm::vec3& size = { 1.0f, 1.0f, 1.0f }, const CollideMode& collider = NONE, const glm::vec3& defColor = { 0.6f, 0.6f, 1.0f });
	float return_hp() { return hp; }

	void HandleCollisionRange(const std::string& group, Model* other) override;
	void HandleCollisionRaycast(const std::string& group, Ray* other);
};