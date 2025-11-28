#pragma once
#include "basic.h"
#include "Standard.h"

class Player; // Player 클래스가 존재함을 알림

// 카메라. Model 클래스 상속으로 getModelMatrix() 사용 가능
class Camera {
private:
	Player* owner = nullptr; // 카메라 클래스를 가지는 플레이어가 누군지 구분하기 위함
	glm::vec3 EYE; // EYE (카메라 위치)
	glm::vec3 AT; // AT (카메라가 바라보는 방향)
	glm::vec3 UP; // UP (카메라의 위쪽)

	// 쿼터니언 방식 카메라 회전
	glm::quat qPitch{ 0, 0, 0, 1 }, qYaw{ 0, 0, 0, 1 }, qRot{ 0, 0, 0, 1 };
	glm::mat4 camRot{ 1.0f };
public:
	// 좌우 회전각, 상하 회전각
	GLfloat yaw = 0.0f, pitch = 0.0f;

	Camera(Player* p);
	void updateCam();
	glm::mat4 retViewMatrix() { return glm::lookAt(EYE, AT, UP); }
	glm::mat4 getRotation();
	glm::mat4 retParentMatrix();
	glm::mat4 getYaw();
};

class Gun : public Model{
	bool temp = false;
public:
	Gun();
};

class Player : public Model {
private:
	glm::vec3 eye{ 0.0f, 0.0f, 0.0f }; // 눈 위치

	// 동적 정보 (값 변경 O)
	glm::mat4 side_rotation = glm::mat4(1.0f); // 좌우 회전
	glm::mat4 up_rotation = glm::mat4(1.0f); // 상하 회전
	bool bounding_onoff = false; // 반동 상태
	float bounding_rotation = 0.0f; // 반동 각도
	static Player* bounding_select; // 반동 적용 대상

	glm::vec3 movement_input = glm::vec3(0.0f, 0.0f, 0.0f); // 축별 이동방향 입력 벡터
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f); // 실제 이동 방향 벡터

	GLfloat last_camera_pitch = 0, last_camera_yaw = 0;
public:
	// 기본 생성자
	Player(glm::vec3 position = { 0.0f, 0.5f, 0.0f }, glm::vec3 scale = { 0.25f, 0.25f, 0.25f });

	bool FPS = false; // 1인칭 모드 여부
	void setFPS(bool state) { FPS = state; } // 1인칭 모드 설정
	glm::vec3 getEye() { return eye; }	// camera에 눈 위치 전달
	glm::mat4 applyCameraRotation(Camera* camera);

	void move(const Direction& dir);
	void stop(const Direction& dir);
	void updateMovement(const GLfloat& deltaTime, Camera* camera);

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
};
