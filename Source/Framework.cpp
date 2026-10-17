#include "pch.h"
#include "Framework.h"

Framework* Framework::current_ = nullptr;

namespace
{
	constexpr int UpdateIntervalMs = 1000 / 240;

	//LegacyGL
	//[윤지훈] 구형 gl함수로 문자열 -> 텍스트 렌더링 해주는 매크로
	void DrawText(const std::string& text, float x, float y, float scale)
	{
		//LegacyGL
		glPushMatrix();
		//LegacyGL
		glTranslatef(x, y, 0.0f);
		//LegacyGL
		glScalef(scale, scale, scale);

		for (unsigned char character : text)
		{
			// GLUT drawing helper used with the legacy fixed-function pipeline here.
			//LegacyGL
			glutStrokeCharacter(GLUT_STROKE_ROMAN, character);
		}

		//LegacyGL
		glPopMatrix();
	}
} // namespace

int Framework::Run(int argc, char** argv, std::unique_ptr<Scene> scene)
{
	current_ = this;

	try
	{
		//[윤지훈]GLUT(창 생성, 화면 출력, 입력처리)시작
		glutInit(&argc, argv);
		//[윤지훈]OpenGL 3.3 ver
		glutInitContextVersion(3, 3);
		// 기존 GLUT 글꼴과 디버그 선은 호환 프로파일에서 그립니다.
		// Compatibility context required by the legacy drawing paths below.
		//LegacyGL
		//[윤지훈]차후 GLUT_CORE_PROFILE로 전환 후 LegacyGL API 다 교체?
		glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
		//[윤지훈]더블버퍼링|RGBA색상모드|깊이테스트사용
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutInitWindowSize(width_, height_);
		glutInitWindowPosition(100, 100);
		window_ = glutCreateWindow("Example1");

		if (window_ <= 0)
		{
			throw std::runtime_error("Cannot create OpenGL window");
		}

		//[윤지훈]창이 닫힐 때 메인루프 함수가 리턴되어 프로그램 즉시 종료하지 않고 다음 코드 실행
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
		glewExperimental = GL_TRUE;

		//[윤지훈]GLEW(최신 그래픽 기능)시작
		if (glewInit() != GLEW_OK)
		{
			throw std::runtime_error("GLEW initialization failed");
		}

		while (glGetError() != GL_NO_ERROR)
		{
		} // GLEW 초기화 과정의 기존 오류 상태 제거

		if (!GLEW_VERSION_3_3)
		{
			throw std::runtime_error("OpenGL 3.3 is required");
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		//[윤지훈]배경 기본색
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		//[윤지훈]포인터 소유권 이전
		scene_ = std::move(scene);
		//[윤지훈]씬 구성하는 모든 요소 생성
		scene_->Initialize();

		//[윤지훈]콜백 함수들 등록하기
		glutDisplayFunc(DisplayCallback);
		glutReshapeFunc(ReshapeCallback);
		glutKeyboardFunc(KeyDownCallback);
		glutKeyboardUpFunc(KeyUpCallback);
		glutMouseFunc(MouseCallback);
		glutMotionFunc(MotionCallback);
		glutPassiveMotionFunc(MotionCallback);
		glutCloseFunc(CloseCallback);
		glutSetCursor(GLUT_CURSOR_NONE);
		previousTime_ = std::chrono::steady_clock::now();
		glutTimerFunc(UpdateIntervalMs, TimerCallback, 0);
		glutMainLoop();
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';
		exitCode_ = 1;
	}

	// OpenGL 컨텍스트가 살아 있을 때 GPU 자원을 먼저 해제합니다.
	scene_.reset();

	if (window_ > 0)
	{
		const int window = window_;
		window_ = 0;
		glutDestroyWindow(window);
	}

	current_ = nullptr;

	return exitCode_;
}

void Framework::Tick()
{
	//[윤지훈]deltatime 구하기
	const auto now = std::chrono::steady_clock::now();
	const float elapsed = std::chrono::duration<float>(now - previousTime_).count();
	previousTime_ = now;

	//[윤지훈]씬 업데이트 -> 오브젝트 업데이트
	scene_->Update(elapsed, input_);
	//[윤지훈]input은 array로, 키 타입을 인덱스로서 갖고 입력 여부를 조사함. 그걸 전부 false로 초기화
	input_.EndFrame();

	if (elapsed > 0.0f)
	{
		glutSetWindowTitle(("FPS: " + std::to_string(static_cast<int>(1.0f / elapsed))).c_str());
	}

	glutPostRedisplay();
}

//[윤지훈] 빛 관련 Uniform 변수 설정 함수
void Framework::UploadLights(const Shader& shader) const
{
	//[윤지훈] 앰비언트 라이트(환경광) 설정
	shader.Set("ambientLight", scene_->ambientLight);
	int count = 0;

	for (const auto& light : scene_->GetLights())
	{
		if (!light.enabled)
		{
			continue;
		}

		//[윤지훈] 빛 개수 제한하여 셰이더의 계산 횟수 제한
		if (count == Light::MaxCount)
		{
			break;
		}

		//[윤지훈] 각 라이트별 정보 설정. 라이트는 셰이더에서 배열로 관리됨
		const std::string prefix = "lights[" + std::to_string(count++) + "].";
		shader.Set(prefix + "type", static_cast<int>(light.type));
		shader.Set(prefix + "color", light.color);
		shader.Set(prefix + "intensity", std::max(light.intensity, 0.0f));
		shader.Set(prefix + "position", light.position);
		shader.Set(prefix + "direction", light.direction);
		shader.Set(prefix + "range", std::max(light.range, 0.0f));
		const float outer = std::clamp(light.outerAngle, 0.1f, 89.0f);
		const float inner = std::clamp(light.innerAngle, 0.0f, outer - 0.1f);
		shader.Set(prefix + "innerCos", std::cos(glm::radians(inner)));
		shader.Set(prefix + "outerCos", std::cos(glm::radians(outer)));
	}

	shader.Set("lightCount", count);
}

void Framework::DrawObjects()
{
	//[윤지훈] 카메라 객체로 행렬 2개 구하기
	const auto& camera = scene_->GetCamera();
	const glm::mat4 projection = camera.GetProjection(static_cast<float>(width_) / height_);
	const glm::mat4 view = camera.GetView();
	const Shader* previousShader = nullptr;

	//[윤지훈] 각 오브젝트마다 수행
	for (const auto& object : scene_->GetObjects())
	{
		//[윤지훈] 비활성 or 투명 객체 스킵
		if (!object->active || !object->visible)
		{
			continue;
		}

		const auto& mesh = object->GetMesh();
		const auto& material = object->GetMaterial();

		if (!mesh || !material || !material->shader)
		{
			continue;
		}

		//[윤지훈] 스케일벡터의 요소 중 하나라도 절댓값이 0.000001보다 작다면 에러. 필요?
		if (glm::any(glm::lessThan(glm::abs(object->transform.scale), glm::vec3(0.000001f))))
		{
			throw std::runtime_error("Object scale must be nonzero: " + object->name);
		}

		//[윤지훈] 머터리얼에 저장된 셰이더 렌더 옵션대로 설정 및 텍스처 바인딩
		material->Apply();
		const Shader& shader = *material->shader;

		if (previousShader != &shader)
		{
			//[윤지훈] 변환행렬 2가지와 빛 계산을 위한 카메라 위치 Uniform 변수 세팅
			shader.Set("view", view);
			shader.Set("projection", projection);
			shader.Set("viewPosition", camera.position);
			UploadLights(shader);
			previousShader = &shader;
		}

		shader.Set("model", object->GetModelMatrix());
		mesh->Draw();
	}
}

//LegacyGL
//[윤지훈] 디버깅용 콜라이더 그리는 함수
void Framework::DrawColliders()
{
	if (!scene_->debugColliders) return;

	// Program 0 selects the fixed-function pipeline in this compatibility context.
	//LegacyGL
	glUseProgram(0);
	//[윤지훈] 컬링 끄고 투영/뷰/월드행렬 설정
	glDisable(GL_CULL_FACE);
	//LegacyGL
	glMatrixMode(GL_PROJECTION);
	const auto projection = scene_->GetCamera().GetProjection(static_cast<float>(width_) / height_);
	//LegacyGL
	glLoadMatrixf(glm::value_ptr(projection));
	//LegacyGL
	glMatrixMode(GL_MODELVIEW);
	const auto view = scene_->GetCamera().GetView();
	//LegacyGL
	glLoadMatrixf(glm::value_ptr(view));
	//LegacyGL
	glColor3f(1, 0, 0);

	for (const auto& object : scene_->GetObjects())
	{
		if (!object->active || object->collider == ColliderType::None)
		{
			continue;
		}

		//LegacyGL
		//[윤지훈] 행렬 정보 투입
		glPushMatrix();

		if (object->collider == ColliderType::Box)
		{
			const auto box = object->GetWorldBox();
			const auto center = (box.min + box.max) * 0.5f;
			const auto size = box.max - box.min;
			//LegacyGL
			//[윤지훈] 이동/스케일 정보 설정
			glTranslatef(center.x, center.y, center.z);
			//LegacyGL
			glScalef(size.x, size.y, size.z);
			// GLUT drawing helper used with the legacy fixed-function pipeline here.
			//LegacyGL
			//[윤지훈] 와이어프레임 큐브 그리기
			glutWireCube(1.0);
		}
		else
		{
			const auto sphere = object->GetWorldSphere();
			//LegacyGL
			glTranslatef(sphere.center.x, sphere.center.y, sphere.center.z);
			// GLUT drawing helper used with the legacy fixed-function pipeline here.
			//LegacyGL
			//[윤지훈] 와이어프레임 구체 그리기
			glutWireSphere(sphere.radius, 12, 8);
		}

		//LegacyGL
		//[윤지훈] 현재 행렬 제거
		glPopMatrix();
	}

	glEnable(GL_CULL_FACE);
}

//LegacyGL
//[윤지훈] HUD 그리는 함수
void Framework::DrawOverlay()
{
	const Hud hud = scene_->GetHud();
	// Program 0 selects the fixed-function pipeline in this compatibility context.
	//LegacyGL
	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//LegacyGL
	glMatrixMode(GL_PROJECTION);
	//LegacyGL
	glLoadIdentity();
	//LegacyGL
	glOrtho(-1, 1, -1, 1, -1, 1);
	//LegacyGL
	glMatrixMode(GL_MODELVIEW);
	//LegacyGL
	glLoadIdentity();

	if (hud.crosshair)
	{
		//LegacyGL
		glColor3f(1, 1, 1);
		glLineWidth(2.0f);
		//LegacyGL
		glBegin(GL_LINES);
		//LegacyGL
		glVertex2f(0.025f, 0);
		//LegacyGL
		glVertex2f(-0.025f, 0);
		//LegacyGL
		glVertex2f(0, 0.025f);
		//LegacyGL
		glVertex2f(0, -0.025f);
		//LegacyGL
		glEnd();
		// 원본은 이후 HUD에도 선 두께 2를 유지합니다.
	}

	//LegacyGL
	glColor3f(0, 0, 0);
	const std::array<glm::vec2, 4> positions{{{-0.99f, 0.9f}, {-0.99f, 0.8f}, {-0.3f, 0.9f}, {0.6f, 0.9f}}};

	for (size_t i = 0; i < hud.lines.size() && i < positions.size(); ++i)
	{
		//[윤지훈] 구형 gl함수로 문자열 -> 텍스트 렌더링 해주는 매크로
		DrawText(hud.lines[i], positions[i].x, positions[i].y, 0.0005f);
	}

	if (!hud.message.empty())
	{
		DrawText(hud.message, -0.6f, -0.15f, hud.messageScale);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void Framework::Render()
{
	if (!scene_) return;

	//[윤지훈] 분류별로 렌더링 후 렌더링 버퍼 스왑
	glViewport(0, 0, width_, height_);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawObjects();
	DrawColliders();
	DrawOverlay();
	glutSwapBuffers();
}

void Framework::Close()
{
	glutLeaveMainLoop();
}

void Framework::Fail(const std::exception& error)
{
	std::cerr << error.what() << '\n';
	exitCode_ = 1;
	Close();
}

void Framework::DisplayCallback()
{
	try
	{
		current_->Render();
	}
	catch (const std::exception& e)
	{
		current_->Fail(e);
	}
}

void Framework::TimerCallback(int)
{
	//[윤지훈] 현재 프레임워크 및 씬이 있을 때에만, 즉 초기화 성공 시에만 타이머 작동
	if (!current_ || !current_->scene_) return;

	try
	{
		current_->Tick();
	}
	catch (const std::exception& e)
	{
		current_->Fail(e);
	}

	//[윤지훈] 재귀적으로 본인 호출
	glutTimerFunc(UpdateIntervalMs, TimerCallback, 0);
}

//[윤지훈] 창 크기 재구성 함수
void Framework::ReshapeCallback(int width, int height)
{
	current_->width_ = std::max(width, 1);
	current_->height_ = std::max(height, 1);
}

//[윤지훈] 키 입력 받으면 아스키코드 값을 인덱스로 사용해서 입력여부 배열에 저장함.
//F1, 방향키 등 입력 받으려면 glutSpecialFunc에 콜백 등록해서 따로 관리해야함
void Framework::KeyDownCallback(unsigned char key, int, int)
{
	if (key == 'q')
	{
		current_->Close();
		return;
	}

	current_->input_.SetKey(key, true);
}

void Framework::KeyUpCallback(unsigned char key, int, int)
{
	current_->input_.SetKey(key, false);
}

//[윤지훈] 마우스 입력. 현재는 좌클릭만 받음
void Framework::MouseCallback(int button, int state, int, int)
{
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	current_->input_.firePressed = true;
}

//[윤지훈] 마우스 움직임
//커서 좌표는 좌상단이 원점. 현재 창의 폭과 높이 절반만큼 빼서 화면 중심 기준으로 좌표 변환
void Framework::MotionCallback(int x, int y)
{
	const int dx = x - current_->width_ / 2, dy = y - current_->height_ / 2;

	if (dx == 0 && dy == 0) return;

	current_->input_.mouseDelta += glm::vec2(dx, dy);
	//[윤지훈] 움직이고 난 뒤 화면 중앙에 커서 복구. UI 구현시 이 부분 조정해야함
	glutWarpPointer(current_->width_ / 2, current_->height_ / 2);
}

//[윤지훈] 시스템 종료 시 콜백
void Framework::CloseCallback()
{
	// 창 X 버튼과 glutLeaveMainLoop 모두 컨텍스트 삭제 전에 이 콜백을 거칩니다.
	current_->scene_.reset();
	current_->window_ = 0;
}
