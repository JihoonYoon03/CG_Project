#pragma once

constexpr float FLOAT_PER_METER = 1.0f; // 1.0f / 1 meter

constexpr float PIXEL_PER_METER = (5.0f);
constexpr float RUN_SPEED_KMPH = 20.0f; // Km / Hour(여기서 현실적인 속도를 결정) (km / h)
constexpr float RUN_SPEED_MPM = (RUN_SPEED_KMPH * 1000.0f / 60.0f); // Meter / Minute
constexpr float RUN_SPEED_MPS = (RUN_SPEED_MPM / 60.0f); // Meter / Second
constexpr float RUN_SPEED_PPS = (RUN_SPEED_MPS * PIXEL_PER_METER); // 초당 몇 픽셀을 이동할지 결졍(PPS) (이것이 속도가 됨)