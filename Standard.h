#pragma once

constexpr float MAX_FPS = 240.0f; // 최대 프레임 제한

constexpr float FLOAT_PER_METER = 1.0f; // 1.0f / 1 meter

constexpr float PIXEL_PER_METER = (5.0f);
constexpr float RUN_SPEED_KMPH = 10.0f; // km/h
constexpr float RUN_SPEED_MPS = (RUN_SPEED_KMPH * 1000.0f / 3600.0f); // m/s

enum Direction {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};