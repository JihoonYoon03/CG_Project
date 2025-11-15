#pragma once
#include "basic.h"

// 기반 클래스
class Objects {
public:
	virtual void Update_Buffer() = 0;
	virtual void draw_shape() = 0;
	virtual ~Objects() {}  // 반드시 virtual 소멸자
};
extern std::vector<Objects*> objects;
//std::vector<Objects*> objects;

// 카메라
class Camera {
private:
	glm::vec3 camera_Pos; // EYE (카메라 위치)
	glm::vec3 camera_Direction; // AT (카메라가 바라보는 방향)
	glm::vec3 camera_Up; // UP (카메라의 위쪽)
	float rotate = 0.0f; // 공전 회전량
public:
	Camera() {
		camera_Pos = glm::vec3(0.0f, 0.0f, 2.0f);
		camera_Direction = glm::vec3(0.0f, 0.0f, 0.0f);
		camera_Up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	void set_camera_Pos(glm::vec3 p) { camera_Pos = p; }
	void set_camera_Direction(glm::vec3 p) { camera_Direction = p; }
	glm::vec3 return_eye() { return camera_Pos; }
	glm::vec3 return_at() { return camera_Direction; }
	glm::vec3 return_up() { return camera_Up; }
	float return_rotate() { return rotate; }
};
extern Camera camera;
//Camera camera;


// 파생 클래스
class Player : public Objects {
private:
	GLuint VAO;
	GLuint VBO_position, VBO_color;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
	// 동적 정보 (값 변경 O)
	glm::vec3 pos; // 이동 (현재 위치)
	glm::mat4 trans_mat; // 복합 변환 행렬
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
	float speed = 0.01f;
public:
	// 기본 생성자
	Player(glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f), float x = 0.25f, float y = 0.25f, float z = 0.25f);
	void Update_Buffer() override;
	void draw_shape() override;
	// 이동
	void up_move();
	void down_move();
	void left_move();
	void right_move();
	// 카메라 위치 세팅
	void camera_pos_setting();
	// 회전량 받아와서 저장
	void rotation(glm::mat4 side, glm::mat4 up);
	// 히트 박스 (좌, 우, 앞, 뒤)
	glm::vec4 return_hitbox();
};