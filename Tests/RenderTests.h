#pragma once
#include "pch.h"
#include "Engine/Scene.h"

// 실제 OpenGL 자원을 로드한 상태에서 사격 및 자원 공유 회귀 검사를 실행합니다.
std::unique_ptr<Scene> CreateRenderTestScene();
