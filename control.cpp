#include "basic.h"
#include "Character.h"

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	// 이동
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
	float max_side_rotate = 180.0f; // 좌우 -180~180도
	float max_updown_rotate = 45.0f; // 상하 -45~45도
	float side, up_down;
	side = -(max_side_rotate / (width / 2)) * (x - width / 2); // 좌우
	up_down = -(max_updown_rotate / (height / 2)) * (y - height / 2); // 위아래
	// 회전 각도가 90도를 넘어가면 상하 회전을 담당하는 x축이 반대로 뒤집히게 된다. 이에 따라 상하 각도도 부호를 뒤집어줄 필요가 있다.
	if (abs(side) >= 90.0f) {
		up_down *= -1;
	}
	
	glm::mat4 xR(1.0f);
	glm::mat4 yR(1.0f);
	xR = glm::rotate(xR, glm::radians(side), glm::vec3(0.0f, 1.0f, 0.0f)); // 회전 변환
	yR = glm::rotate(yR, glm::radians(up_down), glm::vec3(1.0f, 0.0f, 0.0f)); // 회전 변환

	player[0]->rotation(xR, yR);
	player[0]->camera_pos_setting();
	glutPostRedisplay();
}