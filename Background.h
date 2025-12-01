#pragma once
#include "basic.h"


// Model 상속 필요
// Model 안에서 바운딩 박스 출력

// 기반 클래스

class Objects : public Model {
protected:
	float hp = 0;

public:
	Objects(const std::string& filename, const glm::vec3& size = { 1.0f, 1.0f, 1.0f }, const CollideMode& collider = NONE, const glm::vec3& defColor = { 0.6f, 0.6f, 1.0f });
};

