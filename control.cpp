#include "basic.h"
#include "Character.h"

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	// 이동
	case 'w': {
		player[0]->up_move();
		player[0]->camera_setting();
		break;
	}
	case 'a': {
		player[0]->left_move();
		player[0]->camera_setting();
		break;
	}
	case 's': {
		player[0]->down_move();
		player[0]->camera_setting();
		break;
	}
	case 'd': {
		player[0]->right_move();
		player[0]->camera_setting();
		break;
	}
	case 'q': {
		glutLeaveMainLoop();
		break;
	}
	}
	glutPostRedisplay();
}
void Special(int key, int x, int y) {
	glutPostRedisplay();
}

// 마우스 입력에 따른 기능 설정
void Mouse(int button, int state, int x, int y) { // button은 좌클릭과 우클릭 구분, state는 누른 상태인지 뗀 상태인지 구분, x와 y는 각각 '윈도우'에서의 마우스 위치
	if (button == GLUT_LEFT_BUTTON and state == GLUT_DOWN) { // 좌클릭을 눌렀을 때 수행되는 조건문
		player[0]->bounding_on();
	}
	glutPostRedisplay();
}

// 마우스의 화면 위치에 따른 절대값 기반 1인칭 시점
void Passive(int x, int y) {
	float max_side_rotate = 180.0f; // 좌우 -180~180도
	float max_updown_rotate = 45.0f; // 상하 -45~45도

	float side, up_down;

	// 마우스 위치에 따른 회전량 설정
	side = -(max_side_rotate / (width / 2.0f)) * (x - width / 2.0f); // y축 회전이므로 좌회전이 양수, 우회전이 음수
	up_down = -(max_updown_rotate / (height / 2.0f)) * (y - height / 2.0f); // x축 회전이므로 상회전이 양수, 하회전이 음수

	// 최대 각도 설정 (초과 방지)
	if (side < -max_side_rotate)   side = -max_side_rotate;
	if (side > max_side_rotate)   side = max_side_rotate;

	if (up_down < -max_updown_rotate) up_down = -max_updown_rotate;
	if (up_down > max_updown_rotate) up_down = max_updown_rotate;

	// 월드 좌표 기준 y축 회전
	glm::mat4 xR(1.0f);
	xR = glm::rotate(xR, glm::radians(side), glm::vec3(0.0f, 1.0f, 0.0f));

	// 회전 이후의 월드 좌표 기준 x축으로 상하 회전 (월드 좌표계 자체를 회전 시킨 다음 상하 회전을 적용)
	glm::vec3 new_X = glm::normalize(glm::vec3(xR * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	glm::mat4 yR(1.0f);
	yR = glm::rotate(yR, glm::radians(up_down), new_X);

	// 회전 정보 전달
	player[0]->rotation(xR, yR);
	// 카메라 최신화
	player[0]->camera_setting();

	glutPostRedisplay();
}