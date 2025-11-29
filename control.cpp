#include "Control.h"

float mouse_sensitivity = 0.2f;

void Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	// 이동
	case 'w': {
		player->move(FORWARD);
		break;
	}
	case 'a': {
		player->move(LEFT);
		break;
	}
	case 's': {
		player->move(BACKWARD);
		break;
	}
	case 'd': {
		player->move(RIGHT);
		break;
	}
	case '/':
		// 디버그 모드 토글. 해당 변수는 tools.cpp에 정의되어 있음
		debug_mode_collider = !debug_mode_collider;
		break;
	case 'q': {
		glutLeaveMainLoop();
		break;
	}
	}
	glutPostRedisplay();
}

void KeyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': {
		player->stop(FORWARD);
		break;
	}
	case 'a': {
		player->stop(LEFT);
		break;
	}
	case 's': {
		player->stop(BACKWARD);
		break;
	}
	case 'd': {
		player->stop(RIGHT);
		break;
	}
	}
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	glutPostRedisplay();
}

// 월드 업데이트 (매 순간 호출)
void UpdateWorld(int value) {
	calculateFrameTime();
	player->updateMovement(frame_time, camera);
	camera->updateCam();
	if (debugCube) {
		debugCube->update();
	}
	handle_collisions();

	glutTimerFunc(1000 / MAX_FPS, UpdateWorld, 0);
	glutPostRedisplay();
}

// 마우스 입력에 따른 기능 설정
void Mouse(int button, int state, int x, int y) { // button은 좌클릭과 우클릭 구분, state는 누른 상태인지 뗀 상태인지 구분, x와 y는 각각 '윈도우'에서의 마우스 위치
	if (button == GLUT_LEFT_BUTTON and state == GLUT_DOWN) { // 좌클릭을 눌렀을 때 수행되는 조건문
		gun->shoot(camera);
	}
	glutPostRedisplay();
}

void Motion(int x, int y) {
	PassiveMotion(x, y);
}

// 마우스의 화면 위치에 따른 절대값 기반 1인칭 시점
void PassiveMotion(int x, int y) {
	float max_updown_rotate = 89.0f; // 상하 회전 최대 각

	// 마우스 위치 윈도우 좌표계를 OpenGL 좌표계로 변환
	GLfloat gl_x, gl_y;
	mPosToGL(width, height, x, y, gl_x, gl_y);

	// 마우스 위치에 따른 회전량 설정
	camera->yaw -= gl_x * width / 2.0f * mouse_sensitivity; // y축 회전이므로 좌회전이 양수, 우회전이 음수
	camera->pitch += gl_y * height / 2.0f * mouse_sensitivity; // x축 회전이므로 하회전이 양수, 상회전이 음수

	// 값 초기화 & 최대 각도 설정 (초과 방지)
	if (camera->yaw < 0.0f) camera->yaw += 360.0f;
	else if (camera->yaw >= 360.0f) camera->yaw -= 360.0f;

	if (camera->pitch < -max_updown_rotate) camera->pitch = -max_updown_rotate;
	else if (camera->pitch > max_updown_rotate) camera->pitch = max_updown_rotate;

	//// 월드 좌표 기준 y축 회전
	//glm::mat4 xR(1.0f);
	//xR = glm::rotate(xR, glm::radians(camera_yaw), glm::vec3(0.0f, 1.0f, 0.0f));

	//// 회전 이후의 월드 좌표 기준 x축으로 상하 회전 (월드 좌표계 자체를 회전 시킨 다음 상하 회전을 적용)
	//glm::vec3 new_X = glm::normalize(glm::vec3(xR * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	//glm::mat4 yR(1.0f);
	//yR = glm::rotate(yR, glm::radians(camera_pitch), new_X);

	glutWarpPointer(width / 2, height / 2); // 마우스 커서를 윈도우 중앙으로 이동
	glutPostRedisplay();
}