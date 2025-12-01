#pragma once
#include "basic.h"
#include "Variables.h"
#include "UI.h"
#include "Control.h"

// 뷰 및 투영 변환
void view_and_projection() {
	// 투영 변환 행렬 준비
	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 200.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "projection"), 1, GL_FALSE, &pTransform[0][0]); // &pTransform[0][0]의 형식을 통해 mat4 자료형의 view에 대한 시작 주소값을 넘김


	// 뷰 변환 행렬 준비
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(camera->retViewMatrix()));
}

void setLight() {
	glUniform3fv(glGetUniformLocation(shaderProgramID, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shaderProgramID, "lightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(shaderProgramID, "viewPos"), 1, glm::value_ptr(camera->getEYE()));
}

GLvoid drawScene()
{
	glUseProgram(shaderProgramID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view_and_projection();
	setLight();

	// 스카이돔 렌더링
	glDisable(GL_CULL_FACE); // 스카이돔은 안쪽 면을 봐야 하므로 면 제거 비활성화

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skydome->getTexture());

	glUniform1i(glGetUniformLocation(shaderProgramID, "tex"), 0);
	glUniform1i(glGetUniformLocation(shaderProgramID, "useTexture"), 1);
	glUniform1i(glGetUniformLocation(shaderProgramID, "isBackground"), 1);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(skydome->getModelMatrix()));
	skydome->Render();

	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(glGetUniformLocation(shaderProgramID, "useTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgramID, "isBackground"), 0);

	glEnable(GL_CULL_FACE); // 면 제거 다시 활성화

	for (auto& object : objects) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(object->getModelMatrix()));
		object->Render();
	}

	for (auto& target : targets) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(target->getModelMatrix()));
		target->Render();
	}

	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(camera->getRotation()* gun->getModelMatrix() * gun->getAnimationMatrix()));
	gun->Render();
	
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(player->applyCameraRotation(camera) * player->getModelMatrix()));
	player->Render();

	// 디버그 큐브 이동 및 렌더링
	if (debugCube && debug_mode_collider) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(debugCube->getModelMatrix()));
		debugCube->Render();
	}

	draw_UI();

	glutSwapBuffers();
}
