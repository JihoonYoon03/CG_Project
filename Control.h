#pragma once
#include "basic.h"
#include "Variables.h"
#include "Standard.h"
#include "CollideHandler.h"

extern float mouse_sensitivity;
extern Player* player;
extern Camera* camera;

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Special(int key, int x, int y);
void UpdateWorld(int value);

void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void PassiveMotion(int x, int y);