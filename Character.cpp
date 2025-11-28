#include "Character.h"

std::vector<Player*> player;
Camera* camera = nullptr;

Camera::Camera(Player* p) : owner(p) {
	owner->setFPS(true);
	EYE = owner->getEye(this);
	AT = EYE - glm::vec3(0.0f, 0.0f, -1.0f);
	UP = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::updateCam(const GLfloat& camera_pitch, const GLfloat& camera_yaw) {
	EYE = owner->getEye(this);

	// 쿼터니언 방식 카메라 회전
	qPitch = glm::angleAxis(glm::radians(camera_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	qYaw = glm::angleAxis(glm::radians(camera_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	qRot = qYaw * qPitch;
	camRot = glm::mat4_cast(qRot);
	AT = EYE + glm::vec3(camRot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
}

glm::mat4 Camera::getModelMatrix() {
	return glm::translate(glm::mat4(1.0f), EYE) * camRot * glm::translate(glm::mat4(1.0f), -EYE);
}

glm::mat4 Camera::retParentMatrix() {
	return glm::translate(glm::mat4(1.0f), EYE) * camRot * glm::translate(glm::mat4(1.0f), -EYE);
}

glm::mat4 Camera::getYaw() {
	return glm::translate(glm::mat4(1.0f), EYE) * glm::mat4_cast(qYaw) * glm::translate(glm::mat4(1.0f), -EYE);
}

Gun::Gun() : Model("models/Pistol.obj", { 0.00025f, 0.00025f, 0.00025f }, { 0.8f, 0.8f, 0.8f }) {
	rotate({ 0.0f, 180.0f, 0.0f }, center);
	translate({ 0.2f, 0.3f, -0.8f });
}

Player* Player::bounding_select = nullptr; // 클래스 전역 변수 초기화

Player::Player(glm::vec3 position, glm::vec3 scale) : Model("models/Cube.obj", scale, { 0.8f, 0.8f, 0.2f }, BOX) {
	translate(position);
	eye = center + glm::vec3(0.0f, 0.75f, -0.4f);
	gun = new Gun();
	gun->setParent(this);
}

glm::vec3 Player::getEye(Camera* camera) {
	return eye;
}

// FPS모드일 때 카메라 Yaw만 적용
glm::mat4 Player::applyCameraRotation(Camera* camera) {
	if (FPS) {
		return camera->getYaw();
	}
	else {
		return glm::mat4(1.0f);
	}
}

//bool Player::outside_map() {
//	if (return_hitbox()[0] >= objects[0]->return_hitbox()[1]) return true;
//	if (return_hitbox()[1] <= objects[0]->return_hitbox()[0]) return true;
//	if (return_hitbox()[2] >= objects[0]->return_hitbox()[3]) return true;
//	if (return_hitbox()[3] <= objects[0]->return_hitbox()[2]) return true;
//	return false;
//}
//bool Player::collision() {
//	for (size_t i = 1; i < objects.size(); ++i) {
//		if (return_hitbox()[0] <= objects[i]->return_hitbox()[1]
//			and return_hitbox()[1] >= objects[i]->return_hitbox()[0]
//			and return_hitbox()[2] <= objects[i]->return_hitbox()[3]
//			and return_hitbox()[3] >= objects[i]->return_hitbox()[2]) return true;
//	}
//	return false;
//}

// 반동
void Player::bounding_on() { 
	if (bounding_onoff) return; // 이미 반동 중이면 종료
	bounding_onoff = true; 
	bounding_rotation = 10.0f;
	bounding_select = this; // 현재 플레이어가 반동 중임을 알림
	glutTimerFunc(100, bounding_callback, 1);
}
void Player::bounding(int t) {
	if (!bounding_onoff) return;
	bounding_rotation -= 2.0f; // 서서히 각도 감소
	if (bounding_rotation <= 0.0f) {
		bounding_rotation = 0.0f;
		bounding_onoff = false;
	}
	glutTimerFunc(100, bounding_callback, 1);
	glutPostRedisplay();
}
// 타이머에 지속적으로 bounding을 호출하기 위한 방식
// 전역 멤버 함수 내에서 bounding을 대신 호출하고, 이 함수를 타이머에서 계속 반복 호출하는 것
void Player::bounding_callback(int value) {
	if (bounding_select)
		bounding_select->bounding(value);
}