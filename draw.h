#pragma once
#include "basic.h"
#include "Character.h"
#include "Background.h"

// 뷰 및 투영 변환
void view_and_projection() {
	// 셰이더 프로그램에서 view_Transform 변수 위치 view_Location으로 가져오기
	unsigned int view_Location = glGetUniformLocation(shaderProgramID, "view_Transform");
	unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projection_Transform");

	// 뷰 변환 행렬 준비
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 R(1.0f);
	glm::mat4 xR(1.0f);
	xR = glm::rotate(xR, glm::radians(-15.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // x축 회전
	R = glm::rotate(R, glm::radians(camera.return_rotate()), glm::vec3(0.0f, 1.0f, 0.0f)); // 카메라 공전 위치
	glm::vec4 dir = R * xR * glm::vec4(camera.return_eye() - camera.return_at(), 1.0f); // 동차 좌표를 활용해 원점과 pos와의 벡터에다가 회전을 적용
	view = glm::lookAt(camera.return_at() + glm::vec3(dir), camera.return_at(), camera.return_up()); // lookAt을 통한 카메라 설정 (인자값을 통한 카메라 좌표계 생성 후 이에 따른 뷰 변환 행렬을 view에 저장)
	glUniformMatrix4fv(view_Location, 1, GL_FALSE, &view[0][0]); // &view[0][0]의 형식을 통해 mat4 자료형의 view에 대한 시작 주소값을 넘김

	glm::mat4 pTransform = glm::mat4(1.0f); // 투영 변환 행렬 준비
	pTransform = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 200.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]); // &pTransform[0][0]의 형식을 통해 mat4 자료형의 view에 대한 시작 주소값을 넘김
}

GLvoid drawScene()
{
	glUseProgram(shaderProgramID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	frame_work(); // 시간 계산 (이벤트가 작용하여 렌더링 되는 경우 수행)
	view_and_projection();
	for (size_t i = 0; i < objects.size(); ++i) {
		objects[i]->draw_shape();
	}
	player[0]->return_gun().setting_attributes();
	player[0]->return_gun().draw_shape();
	player[0]->draw_shape();
	glutSwapBuffers();
}
