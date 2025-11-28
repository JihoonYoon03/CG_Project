#include "Character.h"

Camera::Camera(Player* p) : owner(p) {
	owner->setFPS(true);
	EYE = owner->getEye();
	AT = EYE - glm::vec3(0.0f, 0.0f, -1.0f);
	UP = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Camera::updateCam() {
	EYE = owner->getEye();

	// 쿼터니언 방식 카메라 회전
	qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	qRot = qYaw * qPitch;
	camRot = glm::mat4_cast(qRot);
	AT = EYE + glm::vec3(camRot * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
}

glm::mat4 Camera::getRotation() {
	return glm::translate(glm::mat4(1.0f), EYE) * camRot * glm::translate(glm::mat4(1.0f), -EYE);
}

glm::mat4 Camera::getYaw() {
	return glm::translate(glm::mat4(1.0f), EYE) * glm::mat4_cast(qYaw) * glm::translate(glm::mat4(1.0f), -EYE);
}

Gun::Gun() : Model("models/Pistol.obj", { 0.00025f, 0.00025f, 0.00025f }, { 0.8f, 0.8f, 0.8f }) {
	rotate({ 0.0f, 180.0f, 0.0f }, center);
	translate({ 0.2f, 0.3f, -0.8f });
}

KeyFrame Gun::getKeyframe() {
	if (!rebound) {
		return { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f) }; // 반동 중이 아니면 기본 위치 반환
	}

	KeyFrame result;
	if (t < 0.05f) {
		float local_t = t / 0.05f; // 0.0f ~ 0.05f 구간을 0.0f ~ 1.0f로 매핑
		result.position = glm::mix(keyframes[0].position, keyframes[1].position, local_t);
		result.rotation = glm::mix(keyframes[0].rotation, keyframes[1].rotation, local_t);
	}
	else {
		float local_t = (t - 0.05f) / 0.95f; // 0.05f ~ 1.0f 구간을 0.0f ~ 1.0f로 매핑
		result.position = glm::mix(keyframes[1].position, keyframes[2].position, local_t);
		result.rotation = glm::mix(keyframes[1].rotation, keyframes[2].rotation, local_t);
	}
	
	t = std::min(t + t_per_time * frame_time, 1.0f); // t 증가, 최대 1.0f까지

	if (t >= 1.0f) {
		t = 0.0f; // 애니메이션 종료 시 초기화
		rebound = false;
	}

	return result;
}

glm::mat4 Gun::getAnimationMatrix() {
	KeyFrame frame = getKeyframe();
	frame.position = glm::inverse(defScaleMatrix) * glm::vec4(frame.position, 0.0f); // 원래 크기 기준으로 변환
	glm::mat4 mat = glm::mat4(1.0f);

	glm::quat qPitch = glm::angleAxis(glm::radians(frame.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat qYaw = glm::angleAxis(glm::radians(frame.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat qRoll = glm::angleAxis(glm::radians(frame.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::quat qRot = qRoll * qYaw * qPitch;

	mat = glm::mat4_cast(qRot);
	mat = glm::translate(mat, frame.position);

	return mat;
}

Player* Player::bounding_select = nullptr; // 클래스 전역 변수 초기화

Player::Player(glm::vec3 position, glm::vec3 scale) : Model("models/Cube.obj", scale, { 0.8f, 0.8f, 0.2f }, BOX) {
	translate(position);
	eye = center + glm::vec3(0.0f, 0.75f, -0.4f);
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


void Player::move(const Direction& dir) {
	switch (dir) {
	case FORWARD:
		if (movement_input.z <= -1.0f) break;
		movement_input.z -= 1.0f;
		break;
	case BACKWARD:
		if (movement_input.z >= 1.0f) break;
		movement_input.z += 1.0f;
		break;
	case LEFT:
		if (movement_input.x <= -1.0f) break;
		movement_input.x -= 1.0f;
		break;
	case RIGHT:
		if (movement_input.x >= 1.0f) break;
		movement_input.x += 1.0f;
		break;
	}
}

void Player::stop(const Direction& dir) {
	switch (dir) {
	case FORWARD:
		if (movement_input.z >= 1.0f) break;
		movement_input.z += 1.0f;
		break;
	case BACKWARD:
		if (movement_input.z <= -1.0f) break;
		movement_input.z -= 1.0f;
		break;
	case LEFT:
		if (movement_input.x >= 1.0f) break;
		movement_input.x += 1.0f;
		break;
	case RIGHT:
		if (movement_input.x <= -1.0f) break;
		movement_input.x -= 1.0f;
		break;
	}
}

void Player::updateMovement(const GLfloat& deltaTime, Camera* camera) {
	if (movement_input == glm::vec3(0.0f, 0.0f, 0.0f)) return; // 이동 입력이 없으면 종료

	direction = glm::vec3(camera->getYaw() * glm::vec4(glm::normalize(movement_input), 0.0f));
	translate(direction * RUN_SPEED_MPS * deltaTime);

	eye = center + glm::vec3(0.0f, 0.75f, -0.4f);
	camera->updateCam();
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