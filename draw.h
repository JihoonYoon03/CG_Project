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

	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	for (size_t i = 0; i < objects.size(); ++i) {
		objects[i]->draw_shape();
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
	if (debugCube) {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(debugCube->getModelMatrix()));
		debugCube->Render();
	}

	draw_UI();

	glutSwapBuffers();
}
