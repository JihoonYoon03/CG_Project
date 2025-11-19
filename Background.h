#pragma once
#include "Character.h"

// 기반 클래스
class Objects {
public:
	virtual void Update_Buffer() = 0;
	virtual void draw_shape() = 0;
	virtual glm::vec4 return_hitbox() = 0;
	virtual ~Objects() {}  // 반드시 virtual 소멸자
};
extern std::vector<Objects*> objects;

class Background : public Objects {
private:
	GLuint VAO;
	GLuint VBO_position, VBO_color;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
	// 동적 정보 (값 변경 O)
	glm::vec3 pos; // 이동 (현재 위치)
	float size_x, size_y, size_z;
public:
	Background(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float x = 5.0f, float y = 5.0f, float z = 5.0f);
	void Update_Buffer() override;
	void draw_shape() override;
	glm::vec4 return_hitbox() override;
};

class Box : public Objects {
private:
	GLuint VAO;
	GLuint VBO_position, VBO_color;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
	// 동적 정보 (값 변경 O)
	glm::vec3 pos; // 이동 (현재 위치)
	float size_x, size_y, size_z;
public:
	Box(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float x = 5.0f, float y = 5.0f, float z = 5.0f);
	void Update_Buffer() override;
	void draw_shape() override;
	glm::vec4 return_hitbox() override;
};

