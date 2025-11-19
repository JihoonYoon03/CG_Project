#pragma once
#include "basic.h"

// 카메라
class Camera {
private:
	glm::vec3 camera_Pos; // EYE (카메라 위치)
	glm::vec3 camera_Direction; // AT (카메라가 바라보는 방향)
	glm::vec3 camera_Up; // UP (카메라의 위쪽)
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
};
extern Camera camera;

class Gun {
	GLuint VAO;
	GLuint VBO_position, VBO_color;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
	// 동적 정보 (값 변경 O)
	glm::vec3 pos; // 이동 (현재 위치)
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
public:
	Gun();
	void Update_Buffer();
	void draw_shape();
	bool loadFromOBJ(const std::string& filename);
	void setting_attributes(); // 위치, 각도 최신화
};

extern float PIXEL_PER_METER; // 10 pixel 30 cm.즉, 1 meter 당 몇 픽셀인지 계산. 10pixel을 0.3(m)으로 나누어 1미터 당 픽셀 수를 구함
extern float RUN_SPEED_KMPH; // Km / Hour(여기서 현실적인 속도를 결정) (km / h)
extern float RUN_SPEED_MPM; // Meter / Minute
extern float RUN_SPEED_MPS; // Meter / Second
extern float RUN_SPEED_PPS; // 초당 몇 픽셀을 이동할지 결졍(PPS) (이것이 속도가 됨)

class Player {
private:
	GLuint VAO;
	GLuint VBO_position, VBO_color;
	std::vector<glm::vec3> v; // 각 Shape의 정점 좌표들을 저장할 벡터 자료구조
	std::vector<glm::vec3> c; // 각 Shape의 정점 색들을 저장할 벡터 자료구조
	// 동적 정보 (값 변경 O)
	glm::vec3 pos; // 이동 (현재 위치)
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
	float speed = RUN_SPEED_PPS;
	float size_x, size_y, size_z;

	Gun gun; // 플레이어에 총 클래스 포함시키기
public:
	// 기본 생성자
	Player(glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f), float x = 0.25f, float y = 0.25f, float z = 0.25f);
	void Update_Buffer();
	void draw_shape();
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
	// 맵 안에 있는지 구분
	bool outside_map();
	bool collision();
	glm::vec3 return_pos() { return pos; }
	glm::mat4 return_side_rotation() { return side_rotation; }
	glm::mat4 return_up_rotation() { return up_rotation; }
	float return_size_x() { return size_x; }
	Gun& return_gun() { return gun; }
};
extern std::vector<Player*> player;