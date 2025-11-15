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

	objects.push_back(new Background(glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, 5.0f, 5.0f));
	objects[0]->Update_Buffer();

	player.push_back(new Player(glm::vec3(0.0f, 0.125f, 0.0f)));
	player[0]->Update_Buffer();
	player[0]->camera_pos_setting();

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