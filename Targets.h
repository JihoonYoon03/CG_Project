#pragma once
#include "basic.h"

class TargetDefault {
protected:
	GLuint VAO;
	GLuint VBO_position, VBO_color, VBO_normal;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
};