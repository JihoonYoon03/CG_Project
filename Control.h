#pragma once
#include "basic.h"
#include "Character.h"
#include "Targets.h"

extern float mouse_sensitivity;
extern GLfloat camera_yaw, camera_pitch;

void Keyboard(unsigned char key, int x, int y);
void Special(int key, int x, int y);

void Mouse(int button, int state, int x, int y);

void Passive(int x, int y);