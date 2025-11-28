#pragma once
#include "basic.h"
#include "Standard.h"

class Player; // Player 클래스가 존재함을 알림

// 키프레임 이동, 회전변환 저장 구조체
struct KeyFrame {
	glm::vec3 position;
	glm::vec3 rotation;	// pitch, yaw, roll
};

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
	// 반동 애니메이션 키프레임. 기본 총 위치에서의 상대 좌표 저장
	KeyFrame keyframes[3] = {
		{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) },
		{ glm::vec3(0.0f, 0.1f, -0.2f), glm::vec3(-15.0f, 0.0f, 0.0f) },
		{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) }
	};

	bool rebound = false; // 반동 애니메이션 진행 상태
	GLfloat t = 0.0f, t_per_time = 1.5f; // 반동 애니메이션 매개변수, 시간 당 증가율
public:
	Gun();
	void shoot() { { rebound = true; t = 0.0f; } } // 반동 애니메이션 시작
	KeyFrame getKeyframe();
	glm::mat4 getAnimationMatrix();
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
};
