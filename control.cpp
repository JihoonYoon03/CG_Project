#include "Control.h"

float mouse_sensitivity = 0.2f;
// yaw - 좌우 회전, pitch - 상하 회전
GLfloat camera_yaw = 0.0f, camera_pitch = 0.0f;

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	// 이동
	case 'w': {
		//player[0]->up_move();
		//player[0]->camera_setting();
		break;
	}
	case 'a': {
		//player[0]->left_move();
		//player[0]->camera_setting();
		break;
	}
	case 's': {
		//player[0]->down_move();
		//player[0]->camera_setting();
		break;
	}
	case 'd': {
		//player[0]->right_move();
		//player[0]->camera_setting();
		break;
	}
	case '/':
		// 디버그 모드 토글
		debug_mode_collider = !debug_mode_collider;
		break;
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
	float max_updown_rotate = 89.0f; // 상하 회전 최대 각

	// 마우스 위치 윈도우 좌표계를 OpenGL 좌표계로 변환
	GLfloat gl_x, gl_y;
	mPosToGL(width, height, x, y, gl_x, gl_y);

	// 마우스 위치에 따른 회전량 설정
	camera_yaw -= gl_x * width / 2.0f * mouse_sensitivity; // y축 회전이므로 좌회전이 양수, 우회전이 음수
	camera_pitch += gl_y * height / 2.0f * mouse_sensitivity; // x축 회전이므로 하회전이 양수, 상회전이 음수

	// 값 초기화 & 최대 각도 설정 (초과 방지)
	if (camera_yaw < 0.0f) camera_yaw += 360.0f;
	else if (camera_yaw >= 360.0f) camera_yaw -= 360.0f;

	if (camera_pitch < -max_updown_rotate) camera_pitch = -max_updown_rotate;
	else if (camera_pitch > max_updown_rotate) camera_pitch = max_updown_rotate;

	//// 월드 좌표 기준 y축 회전
	//glm::mat4 xR(1.0f);
	//xR = glm::rotate(xR, glm::radians(camera_yaw), glm::vec3(0.0f, 1.0f, 0.0f));

	//// 회전 이후의 월드 좌표 기준 x축으로 상하 회전 (월드 좌표계 자체를 회전 시킨 다음 상하 회전을 적용)
	//glm::vec3 new_X = glm::normalize(glm::vec3(xR * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	//glm::mat4 yR(1.0f);
	//yR = glm::rotate(yR, glm::radians(camera_pitch), new_X);

	player[0]->camera->updateCam(camera_pitch, camera_yaw);

	glutWarpPointer(width / 2, height / 2); // 마우스 커서를 윈도우 중앙으로 이동
	glutPostRedisplay();
}