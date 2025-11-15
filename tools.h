#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <queue>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

struct ColoredVertex {
	glm::vec3 pos;
	glm::vec3 color;
};

class DisplayBasis;

/*
Model 클래스 사용법

1.	생성자 인자
	(모델 들어있는 파일 경로, 크기, 기본 색상)
	크기와 기본 색상은 생략 가능 (기본값: (1,1,1), (0.8,0.8,0.8))
	크기 설정은 정점 데이터를 바꾸는 것이기에 영구적으로 적용됨

2.	모델 변환 함수들
	setDef~(): modelMatrix(=월드 변환) 적용 이전에 먼저 적용할 변환 설정 (무조건 SRT 순서로 적용됨)
	scale(), rotate(), translate(): transformQueue(변환 큐)에 변환 행렬 추가, 추가하는 순서대로 변환 적용됨
	setParent(): 부모 모델을 설정함. 사용 안할 시에는 본인에 대한 변환만, 설정 시엔 부모 모델의 변환도 함께 적용됨
	getModelMatrix(): modelMatrix에 변환 큐 안의 변환 행렬들을 모두 적용한 뒤 부모 모델이 있다면 부모 모델 변환도 적용 후 반환

3.	활성 상태
	enabled 상태가 활성 여부임. 비활성 상태이면 비활성 이후 변환과 렌더링 모두 작동 안함
*/

class Model {
	// 정점 속성
	std::vector<glm::vec3> vertices;	// 정점 위치
	std::vector<glm::uvec3> faces;		// 삼각형의 정점 인덱스 순서
	std::vector<glm::vec3> normals;		// 정점 법선 벡터

	std::vector<glm::vec3>* color;		// 기본 정점 색상, 텍스쳐 적용 할 경우 불필요

	glm::vec3 center;		// 모델 중심점 (정점 좌표값의 최대/최소값 기준 중앙임)
	DisplayBasis* basis;	// 필요 시 모델 중심 좌표계 표시용 (디버그용)


	// 변환행렬 적용 이전에 SRT순으로 모델 변환
	glm::mat4 default_scale = glm::mat4(1.0f);
	glm::mat4 default_rotate = glm::mat4(1.0f);
	glm::mat4 default_translate = glm::mat4(1.0f);

	// 모델 변환
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	// modelMatrix에 적용할 변환 행렬 큐
	std::queue<glm::mat4> transformQueue;

	// 변환 기준이 되는 부모 모델 (nullptr이면 적용 X)
	Model* parent = nullptr;

	// OpenGL 버퍼 객체
	GLuint VAO, VBO, EBO, COLOR, NORMAL;

	// 비활성 상태에선 동작 X
	bool enabled = true;
public:
	Model(const std::string& filename, const glm::vec3& size = { 1.0f, 1.0f, 1.0f }, const glm::vec3& defColor = { 0.8f, 0.8f, 0.8f });

	void setParent(Model* parent);

	void setDefScale(const glm::vec3& ds);
	void setDefRotate(const glm::mat4& dr);
	void setDefTranslate(const glm::vec3& dt);
	
	void scale(const glm::vec3& scaleFactor);
	void rotate(GLfloat angle, const glm::vec3& axis);
	void translate(const glm::vec3& delta);

	void Render();
	void resetModelMatrix();
	glm::vec3 retDistTo(const glm::vec3& origin = { 0.0f, 0.0f, 0.0f });
	glm::mat4 getModelMatrix();
	glm::vec3 retCenter() const { return center; }

	void setEnabled(bool state) { enabled = state; }
	
	~Model();
};

class DisplayBasis {
	glm::vec3 origin;
	ColoredVertex xyz[3][2];

	GLuint VAO, VBO;
public:
	DisplayBasis(GLfloat offset = 1.0f, const glm::vec3& origin = { 0.0f, 0.0f, 0.0f });
	void Render();
};

void mPosToGL(GLuint winWidth, GLuint winHeight, int mx, int my, GLfloat& xGL, GLfloat& yGL);

std::string read_file(const std::string& filename);
void make_vertexShaders(GLuint& vertexShader, const std::string& shaderName);
void make_fragmentShaders(GLuint& fragmentShader, const std::string& shaderName);
GLuint make_shaderProgram(const GLuint& vertexShader, const GLuint& fragmentShader);