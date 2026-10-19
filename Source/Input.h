#pragma once
#include "pch.h"

//[윤지훈] 키 입력여부 기록하는 클래스
// 매 Tick마다 EndFrame 호출되어 pressed 초기화됨
// key는 키를 누를 때/뗄 때에만 상태가 바뀜. pressed는 입력 여부 한 번만 기록하고 초기화됨
// Framework가 채우고 Scene가 읽습니다. pressed와 mouseDelta는 업데이트 후 초기화합니다.
class Input
{
public:
	void SetKey(unsigned char key, bool down)
	{
		if (down)
		{
			pressed_[key] = true;
		}

		keys_[key] = down;
	}

	bool IsDown(unsigned char key) const { return keys_[key]; }
	bool WasPressed(unsigned char key) const { return pressed_[key]; }

	void EndFrame()
	{
		pressed_.fill(false);
		mouseDelta = glm::vec2(0.0f);
		firePressed = false;
	}

	void Reset()
	{
		keys_.fill(false);
		EndFrame();
	}

	glm::vec2 mouseDelta{0.0f};
	bool	  firePressed = false;

private:
	std::array<bool, 256> keys_{};
	std::array<bool, 256> pressed_{};
};
