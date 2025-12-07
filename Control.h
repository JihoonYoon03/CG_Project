#pragma once
#include "basic.h"
#include "Variables.h"
#include "Standard.h"
#include "CollideHandler.h"
#include "Targets.h"

extern float mouse_sensitivity;
extern Player* player;
extern Camera* camera;

extern bool stage_onoff;
extern bool start_stage_onoff;
extern bool over_stage_onoff;
extern bool end_stage_onoff;
extern float timer_value;

void Keyboard(unsigned char key, int x, int y);
void KeyboardUp(unsigned char key, int x, int y);
void Special(int key, int x, int y);
void UpdateWorld(int value);

void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void PassiveMotion(int x, int y);

void start_stage(int t);
void over_stage(int t);
void stage1(int t);
void stage2(int t);
void stage3(int t);