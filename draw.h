#pragma once
#include "basic.h"
#include "Character.h"
#include "Background.h"
#include "UI.h"

// 뷰 및 투영 변환
void view_and_projection() {
	// 셰이더 프로그램에서 view_Transform 변수 위치 view_Location으로 가져오기
	unsigned int view_Location = glGetUniformLocation(shaderProgramID, "view_Transform");
	unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projection_Transform");

	// 뷰 변환 행렬 준비
	glm::mat4 view = glm::lookAt(camera.return_eye(), camera.return_at(), camera.return_up());
	glUniformMatrix4fv(view_Location, 1, GL_FALSE, &view[0][0]);

	// 투영 변환 행렬 준비
	glm::mat4 pTransform = glm::mat4(1.0f);
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
	for (auto& target : targets) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model_Transform"), 1, GL_FALSE, glm::value_ptr(target.getModelMatrix()));
		target.Render();
	}
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model_Transform"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	player[0]->return_gun().setting_attributes();
	player[0]->return_gun().draw_shape();
	player[0]->draw_shape();
    draw_UI();
	glutSwapBuffers();
}
