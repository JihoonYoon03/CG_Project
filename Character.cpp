#include "Character.h"

std::vector<Player*> player;

Gun::Gun(Player* p) : Model("models/Pistol.obj", { 0.00025f, 0.00025f, 0.00025f }, { 0.8f, 0.8f, 0.8f }), owner(p) {
	rotate({ 0.0f, 180.0f, 0.0f }, center);
}

void Gun::setting_attributes() {
	// 카메라 외부 파라미터
	glm::vec3 eye = owner->return_camera().return_eye();
	glm::vec3 at = owner->return_camera().return_at();
	glm::vec3 up = owner->return_camera().return_up();

	// 카메라 방향 벡터들 (u, v, n)
	glm::vec3 forward_V = glm::normalize(at - eye); // at - eye를 통해 카메라가 바라보고 있는 방향에 대한 벡터가 결정되므로, 회전이 적용되어있다 봐도 무방
	glm::vec3 right_V = glm::normalize(glm::cross(forward_V, up));
	glm::vec3 up_V = glm::cross(right_V, forward_V);

	// 총 위치 (카메라 공간)
	glm::vec3 gunOffset(0.125f, -0.35f, 0.15f);

	// 총이 각 카메라 축 방향으로 이동한 것을 모두 합쳐 최종 이동 벡터를 만듦 (이 최종 이동 벡터가 월드 공간 내 총의 위치를 가리키게 됨)
	// 즉, 총의 카메라 공간 위치만큼 카메라 공간의 기저 벡터를 스케일하고 이를 모두 합하여 원하는 방향을 가리키는 하나의 벡터로 만들어주는 것
	glm::vec3 worldOffset =
		right_V * gunOffset.x
		+ up_V * gunOffset.y
		+ forward_V * gunOffset.z;

	if (!temp) {
		this->translate(eye + worldOffset); // 위에서 구한 벡터를 카메라 위치에 더하여 총의 최종 월드 좌표를 구함
		temp = true;
	}

	// 플레이어로부터 회전 정보 받아옴
	side_rotation = player[0]->return_side_rotation();
	up_rotation = player[0]->return_up_rotation();
}


Player* Player::bounding_select = nullptr; // 클래스 전역 변수 초기화

Player::Player(glm::vec3 position, glm::vec3 scale) : Model("models/Cube.obj", scale, { 0.8f, 0.8f, 0.2f }, BOX), gun(this), camera(this) {}

// 카메라 세팅
void Player::camera_setting() {
	glm::mat4 u = up_rotation;
	if (bounding_onoff) {
		glm::vec3 new_X = glm::normalize(glm::vec3(side_rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		glm::mat4 yR(1.0f);
		yR = glm::rotate(yR, glm::radians(bounding_rotation), new_X);
		u = yR * up_rotation;
	}
	// 1. 캐릭터의 전방을 side_rotation으로 돌려서 실제 전방을 구함 (어차피 방향만 주면 되기에 정면은 그냥 glm::vec3 baseFront(0.0f, 0.0f, -1.0f)로 처리)
	glm::vec3 baseFront(0.0f, 0.0f, -1.0f);
	glm::vec3 forward = glm::normalize(glm::vec3(u * side_rotation * glm::vec4(baseFront, 0.0f))); // 정면을 side_rotation만큼 회전 시킨 후 정규화 (안정성을 위한 정규화) (방향만 쓰고 이동은 고려하지 않는 방향 벡터이기에 동차좌표 0.0f로 사용)
	// 캐릭터 중심점 (카메라 위치 및 회전 시 몸체가 보이지 않도록 하는 높이를 고려해 조금 위쪽으로 설정)
	glm::vec3 pos = glm::vec3(center.x, center.y + 0.8f, center.z);
	// 캐릭터로부터 카메라를 둘 위치(머리)
	camera.set_camera_Pos(pos + forward * 0.125f);
	// 카메라가 정면을 바라봄
	camera.set_camera_Direction(pos + forward);
}
// 회전량 받아와서 저장
void Player::rotation(glm::mat4 side, glm::mat4 up) {
	side_rotation = side;
	up_rotation = up;
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
	camera_setting();
	gun.setting_attributes();
	glutTimerFunc(100, bounding_callback, 1);
	glutPostRedisplay();
}
// 타이머에 지속적으로 bounding을 호출하기 위한 방식
// 전역 멤버 함수 내에서 bounding을 대신 호출하고, 이 함수를 타이머에서 계속 반복 호출하는 것
void Player::bounding_callback(int value) {
	if (bounding_select)
		bounding_select->bounding(value);
}