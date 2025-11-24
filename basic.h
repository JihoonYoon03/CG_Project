#pragma once
#include "tools.h"
extern std::random_device dre;

// 윈도우 크기
extern GLint width, height;
//--- 세이더 프로그램 이름
extern GLuint shaderProgramID;
//--- 버텍스세이더객체
extern GLuint vertexShader;
//--- 프래그먼트 세이더객체
extern GLuint fragmentShader;

extern float frame_time;
extern std::chrono::steady_clock::time_point current_time;

void Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Special(int key, int x, int y);
void Passive(int x, int y);
void Reshape(int w, int h);

char* filetobuf(const char* file);

// 버텍스 셰이더 객체 만들기
void basic_make_vertexShaders();
//--- 프래그먼트 세이더 객체 만들기
void basic_make_fragmentShaders();
// 셰이더 프로그램
GLuint basic_make_shaderProgram();

// 시간 계산
void frame_work();