#include "Control.h"

float mouse_sensitivity = 0.2f;

bool stage_onoff = false;
bool start_stage_onoff = false;
bool end_stage_onoff = false;
float timer_value = 0.0f;

void reset_values(int stage) {
	stage_onoff = true;
	start_stage_onoff = true;
	end_stage_onoff = false;
	timer_value = 0.0f;
	stage_level = stage;
	point_count = 0;
	timer_count = 10;
	for (auto t : targets)
		t->turn_off();
}

void make_target() {
	// 타깃 오브젝트 활성화
	std::vector<TargetDefault*> offTargets; // 비활성화 된 타겟을 담을 자료구조

	for (auto* t : targets)
		if (!t->return_enabled())
			offTargets.push_back(t); // 비활성화 된 타겟이라면 추가

	if (offTargets.empty()) return; // 모두 활성화 되었다면 종료

	std::uniform_int_distribution<int> rand_index(0, offTargets.size() - 1); // 비활성화된 것들 중 하나 선택
	offTargets[rand_index(dre)]->turn_on(); // 활성화
}

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
	case '1': {
		if (stage_onoff) return;
		reset_values(1);
		glutTimerFunc(frame_time * 1000, start_stage, 1);
		break;
	}
	case '2': {
		if (stage_onoff) return;
		reset_values(2);
		glutTimerFunc(frame_time * 1000, start_stage, 2);
		break;
	}
	case '3': {
		if (stage_onoff) return;
		reset_values(3);
		glutTimerFunc(frame_time * 1000, start_stage, 3);
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


void start_stage(int t) {
	if (timer_value >= 2) {
		start_stage_onoff = false; // 스테이지 시작을 알리는 텍스트를 끄고 오브젝트 생성 타이머 시작
		end_stage_onoff = false;
		timer_value = 0.0f;
		if (t == 1)	glutTimerFunc(frame_time * 1000, stage1, 0);
		else if (t == 2) glutTimerFunc(frame_time * 1000, stage2, 0);
		else if (t == 3) glutTimerFunc(frame_time * 1000, stage3, 0);
		else if (t == 4) {
			timer_value = 0.0f;
			end_stage_onoff = true;
			stage_onoff = false;
			point_count = 0;
			for (auto t : targets)
				t->turn_off();
			glutTimerFunc(frame_time * 1000, start_stage, 0);
		}
		return;
	}
	timer_value += frame_time;
	glutTimerFunc(frame_time * 1000, start_stage, t);
}
void stage1(int t) {
	if (timer_value >= 1) {
		make_target();
		timer_count--;
		timer_value = 0;
	}
	else timer_value += frame_time;

	if (timer_count == 0) { // 다음 스테이지 준비
		reset_values(2);
		glutTimerFunc(frame_time * 1000, start_stage, 2);
		return; 
	}
	glutTimerFunc(frame_time * 1000, stage1, 0);
}
void stage2(int t) {
	if (timer_value >= 1) {
		make_target();
		make_target();
		timer_count--;
		timer_value = 0;
	}
	else timer_value += frame_time;

	if (timer_count == 0) { // 다음 스테이지 준비
		reset_values(3);
		glutTimerFunc(frame_time * 1000, start_stage, 3);
		return;
	}
	glutTimerFunc(frame_time * 1000, stage2, 0);
}
void stage3(int t) {
	if (timer_value >= 1) {
		make_target();
		make_target();
		make_target();
		timer_count--;
		timer_value = 0;
	}
	else timer_value += frame_time;

	if (timer_count == 0) { // 다음 스테이지 준비
		glutTimerFunc(frame_time * 1000, start_stage, 4);
		return;
	}
	glutTimerFunc(frame_time * 1000, stage3, 0);
}