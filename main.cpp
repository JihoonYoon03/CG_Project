#include "basic.h"
#include "Variables.h"
#include "draw.h"
#include "CollideHandler.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma warning(disable: 4711 4710 4100)

//--- 메인 함수
int main(int argc, char** argv)
{
	width = 800;
	height = 600;
	//--- 윈도우생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	basic_make_vertexShaders();
	basic_make_fragmentShaders();
	shaderProgramID = basic_make_shaderProgram();

	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutPassiveMotionFunc(PassiveMotion);
	glutSpecialFunc(Special);
	glutTimerFunc(1000 / MAX_FPS, UpdateWorld, 0); // 월드 업데이트 (매 순간 호출)

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 배경 흰색
	glEnable(GL_DEPTH_TEST); // 기본 깊이 검사 활성화
	glFrontFace(GL_CCW); // 시계 반대 방향을 앞면으로 설정
	glEnable(GL_CULL_FACE); // 뒷면 제거 활성화
	glutSetCursor(GLUT_CURSOR_NONE); // 마우스 커서 숨기기

	// 배경
	// 스카이돔 배경 이미지 출처
	// https://polyhaven.com/a/kloofendal_48d_partly_cloudy_puresky

	skydome = new Model("models/Sphere.obj", glm::vec3(150.0f), glm::vec3(1.0f, 1.0f, 1.0f), NONE, "models/SkydomeTex.png");

	// 배경
	objects.push_back(new Objects("models/Cube.obj", glm::vec3(5.0f, 0.0f, 5.0f), BOX)); // 바닥
	add_collision_pair_BB("player:floor", nullptr, objects.back());
	objects.push_back(new Objects("models/Cube.obj", glm::vec3(5.0f, 0.125f, 0.1f), BOX)); // 울타리
	objects.back()->translate(glm::vec3(0.0f, 0.0625f, -2.5f));
	add_collision_pair_BB("player:fance", nullptr, objects.back());

	// 플레이어
	player = new Player(glm::vec3(0.0f, 1.0f, 1.0f));
	add_collision_pair_BB("player:floor", player, nullptr);
	add_collision_pair_BB("player:fance", player, nullptr);

	gun = new Gun();
	gun->setParent(player);

	// 카메라
	camera = new Camera(player);

	// 타깃 오브젝트
	std::uniform_real_distribution<float> rand_x(-5.0f, 5.0f);
	std::uniform_real_distribution<float> rand_y(0.5f, 2.0f);
	std::uniform_real_distribution<float> rand_z(-5.0f, -0.5f);
	for (int i = 0; i < 10; i++) {
		targets.push_back(new TargetDefault("models/Sphere.obj", glm::vec3(0.15f, 0.15f, 0.15f), SPHERE));
		targets.back()->translate(glm::vec3(rand_x(dre), rand_y(dre), rand_z(dre)));
		add_collision_pair_raycast("bullet:target", nullptr, targets.back());
	}

	glutMainLoop();

	// new로 만든 오브젝트 delete로 초기화
	for (auto obj : objects)
		delete obj;
	objects.clear();
	for (auto t : targets)
		delete t;
	targets.clear();

	delete player;
	delete gun;
	delete camera;
	return 0;
}