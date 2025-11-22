#pragma once
#include "Character.h"

void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);  // 텍스트 시작 위치

    // 문자열 문자 하나씩 그리기
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        ++text;
    }
}

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

    // ===================================
    // 레거시 함수 수정 필요!!!
    glBegin(GL_LINES);
    glVertex2f(+size, 0.0f);
    glVertex2f(-size, 0.0f);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(0.0f, +size);
    glVertex2f(0.0f, -size);
    glEnd();
	// ===================================

    // 셰이더 비활성화 (텍스트 출력을 위함)
    glUseProgram(0);
    // 텍스트
    int stage_level = 1; // 나중에 스테이지 시스템 추가하면 해당 값 받아와서 넣으면 됨
    std::string stage = std::to_string(stage_level);
    stage = "stage: " + stage;
    int point_count = 0; // 나중에 점수 시스템 추가하면 해당 값 받아와서 넣으면 됨
    std::string point = std::to_string(point_count);
    point = "point: " + point; 
    int timer_count = 0; // 나중에 타이머 시스템 추가하면 해당 값 받아와서 넣으면 됨
    std::string timer = std::to_string(timer_count);
    timer = "timer: " + timer;
    glColor3f(0.0f, 0.0f, 0.0f); // 글꼴 검정으로 변경
    drawText(-0.99f, 0.9f, stage.c_str());
    drawText(-0.99f, 0.8f, point.c_str());
    drawText(+0.8f, 0.9f, timer.c_str());

    // 깊이 검사 재활성화
    glEnable(GL_DEPTH_TEST);
    // 셰이더 재활성화
    glUseProgram(shaderProgramID);
}