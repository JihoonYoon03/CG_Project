#pragma once
#include "Character.h"

void draw_UI() {
    // 2d 공간에 그리기 위해 화면에 대한 투영 변환 행렬 등 모두 초기화
    unsigned int view_Location = glGetUniformLocation(shaderProgramID, "view_Transform");
    unsigned int projLoc = glGetUniformLocation(shaderProgramID, "projection_Transform");
    unsigned int modelLoc = glGetUniformLocation(shaderProgramID, "model_Transform");
    glm::mat4 m(1.0f);
    glUniformMatrix4fv(view_Location, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m[0][0]);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m[0][0]);

    // 깊이 검사 비활성화 (항상 맨 위에 그려지도록)
    glDisable(GL_DEPTH_TEST);

    // 십자선 그리기
    glLineWidth(2.0f); // 두께
    float size = 0.025f; // 크기
    glBegin(GL_LINES);
    glVertex2f(+size, 0.0f);
    glVertex2f(-size, 0.0f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0.0f, +size);
    glVertex2f(0.0f, -size);
    glEnd();

    // 깊이 검사 재활성화
    glEnable(GL_DEPTH_TEST);
}