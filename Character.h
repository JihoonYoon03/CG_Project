#pragma once
#include "basic.h"
#include "Background.h"

class Player; // Player 클래스가 존재함을 알림

// 카메라
class Camera {
private:
	Player* owner = nullptr; // 카메라 클래스를 가지는 플레이어가 누군지 구분하기 위함
	glm::vec3 camera_Pos; // EYE (카메라 위치)
	glm::vec3 camera_Direction; // AT (카메라가 바라보는 방향)
	glm::vec3 camera_Up; // UP (카메라의 위쪽)
public:
	Camera(Player* p) : owner(p) {
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

class Gun : public Model{
	bool temp = false;
	Player* owner = nullptr; // 총을 소유한 플레이어
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
public:
	Gun(Player *p);
	void Update_Buffer();
	void draw_shape();
	void setting_attributes(); // 위치, 각도 최신화
};

class Player : public Model {
private:
	// 동적 정보 (값 변경 O)
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
	bool bounding_onoff = false; // 반동 상태
	float bounding_rotation = 0.0f; // 반동 각도
	static Player* bounding_select; // 반동 적용 대상

	GLfloat speed = 0.1f; // 이동 속도

	Gun gun; // 플레이어에 총 클래스 포함시키기
	Camera camera;
public:
	// 기본 생성자
	Player(glm::vec3 position = { 0.0f, 0.5f, 0.0f }, glm::vec3 scale = { 0.25f, 0.25f, 0.25f });
	void Update_Buffer();
	void draw_shape();
	// 카메라 세팅
	void camera_setting();
	// 회전량 받아와서 저장
	void rotation(glm::mat4 side, glm::mat4 up);
	// 히트 박스 (좌, 우, 앞, 뒤)
	glm::vec4 return_hitbox();
	// 맵 안에 있는지 구분
	//bool outside_map();
	//bool collision();
	glm::mat4 return_side_rotation() { return side_rotation; }
	glm::mat4 return_up_rotation() { 
		glm::mat4 u = up_rotation;
		if (bounding_onoff) { // 반동 중이면 반동이 적용된 상하 각도로 총에 전달
			glm::vec3 new_X = glm::normalize(glm::vec3(side_rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
			glm::mat4 yR(1.0f);
			yR = glm::rotate(yR, glm::radians(bounding_rotation), new_X);
			u = yR * up_rotation;
		}
		return u;
	}
	bool return_bounding() { return bounding_onoff; }
	void bounding_on();
	static void bounding_callback(int value);
	void bounding(int t);
	Gun& return_gun() { return gun; }
	Camera& return_camera() { return camera; }
};
extern std::vector<Player*> player;