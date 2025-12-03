#pragma once
#include "basic.h"


// Model 상속 필요
// Model 안에서 바운딩 박스 출력

// 기반 클래스

class Objects : public Model {
protected:
	glm::vec3 pos;

public:
	Objects(const std::string& filename, const glm::vec3& pos = { 0.0f, 0.0f, 0.0f }, const glm::vec3 & size = { 1.0f, 1.0f, 1.0f }, const CollideMode & collider = NONE, const glm::vec3 & defColor = { 0.6f, 0.6f, 1.0f }, const std::string& texture = "");
	glm::vec3 return_pos() { return pos; }
};

