#include "basic.h"
#include "Character.h"

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': {
		player[0]->up_move();
		player[0]->camera_pos_setting();
		break;
	}
	case 'a': {
		player[0]->left_move();
		player[0]->camera_pos_setting();
		break;
	}
	case 's': {
		player[0]->down_move();
		player[0]->camera_pos_setting();
		break;
	}
	case 'd': {
		player[0]->right_move();
		player[0]->camera_pos_setting();
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

// 마우스의 화면 위치에 따른 절대값 기반 1인칭 시점
void Passive(int x, int y) {
	float max_rotate = 90.0f; // (상하좌우 모두 -90 ~ 90의 범위를 가짐)
	// 좌우 자전
	float side = -(max_rotate / (width / 2)) * (x - width / 2); // 좌우
	glm::mat4 xR(1.0f);
	xR = glm::rotate(xR, glm::radians(side), glm::vec3(0.0f, 1.0f, 0.0f)); // 회전 변환

	// 상하 자전
	float up_down = -(max_rotate * 0.5f / (height / 2)) * (y - height / 2); // 위아래
	glm::mat4 yR(1.0f);
	yR = glm::rotate(yR, glm::radians(up_down), glm::vec3(1.0f, 0.0f, 0.0f)); // 회전 변환

	player[0]->rotation(xR, yR);
	player[0]->camera_pos_setting();
	glutPostRedisplay();
}