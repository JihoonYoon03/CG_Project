#include "basic.h"
#include "Character.h"
#include "Background.h"
#include "draw.h"

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
	glutSpecialFunc(Special);
	glutPassiveMotionFunc(Passive);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // 배경 흰색
	glEnable(GL_DEPTH_TEST); // 기본 깊이 검사 활성화
	glutIdleFunc(frame_work); // 시간 계산 (아무런 이벤트도 없는 상황일 때 수행) (아무런 동작이 없을 때 생기는 drawScene에서의 시간 계산 공백 부분 보완)

	// 바닥
	objects.push_back(new Background(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, 5.0f, 5.0f));
	objects[0]->Update_Buffer();

	// 박스
	objects.push_back(new Box(glm::vec3(0.0f, 0.0625f, 0.0f), 5.0f, 0.125f, 0.1f));
	objects[1]->Update_Buffer();

	// 플레이어
	player.push_back(new Player(glm::vec3(0.0f, 0.125f, 1.0f)));
	player[0]->Update_Buffer();
	player[0]->camera_pos_setting(); // 카메라 조정

	glutMainLoop();

	// new로 만든 오브젝트 delete로 초기화
	for (auto obj : objects)
		delete obj;
	objects.clear();
	for (auto p : player)
		delete p;
	player.clear();
	return 0;
}