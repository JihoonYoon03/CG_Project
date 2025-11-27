#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <queue>
#include <chrono>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

extern bool debug_mode_collider;

struct ColoredVertex {
	glm::vec3 pos;
	glm::vec3 color;
};

class DisplayBasis;

class BoxCollider;
class SphereCollider;

enum CollideMode{
	NONE	=	0b00000000,
	BOX		=	0b00000001,
	SPHERE	=	0b00000010
};

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
// 콜라이더는 해당 객체에 종속되므로 private
// Model의 자식 클래스에선 접근 불가
private:
	BoxCollider* bounding_box = nullptr;
	SphereCollider* bounding_sphere = nullptr;
protected:
	// 정점 속성
	std::vector<glm::vec3> vertices;	// 정점 위치
	std::vector<glm::uvec3> faces;		// 삼각형의 정점 인덱스 순서
	std::vector<glm::vec3> normals;		// 정점 법선 벡터

	std::vector<glm::vec3>* color;		// 기본 정점 색상, 텍스쳐 적용 할 경우 불필요

	glm::vec3 center { 0, 0, 0 };		// 모델 중심점 (정점 좌표값의 최대/최소값 기준 중앙임)

	// 모델 변환
	glm::mat4 defScaleMatrix = glm::mat4(1.0f);	// 모델 기본 크기 조정
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	// modelMatrix에 적용할 변환 행렬 큐
	std::queue<glm::mat4> transformQueue;

	// 변환 기준이 되는 부모 모델 (nullptr이면 적용 X)
	Model* parent = nullptr;

	// OpenGL 버퍼 객체
	GLuint VAO, VERTEX, FACE, COLOR, NORMAL;

	// 비활성 상태에선 동작 X
	bool enabled = true;
public:
	Model() { enabled = false; }
	Model(const std::string& filename, const glm::vec3& size = { 1.0f, 1.0f, 1.0f }, const glm::vec3& defColor = { 0.8f, 0.8f, 0.8f }, const CollideMode& collider = NONE);

	void setParent(Model* parent);

	// origin 기준 변환
	// origin = retCenter()로 사용하면 모델 중심 기준 변환
	void scale(const glm::vec3& ds, const glm::vec3& origin = {0, 0, 0});
	void rotate(const glm::vec3& dr, const glm::vec3& origin = { 0, 0, 0 });
	void translate(const glm::vec3& dt);

	virtual void Render();
	void resetModelMatrix();
	glm::vec3 retDistTo(const glm::vec3& origin = { 0.0f, 0.0f, 0.0f });

	virtual glm::mat4 getModelMatrix();		// 리턴값 커스텀 가능
	virtual glm::mat4 retParentMatrix();	// 리턴값 커스텀 가능

	void setEnabled(bool state) { enabled = state; }

	~Model() {
		delete color;
		if (bounding_box != nullptr) {
			delete bounding_box;
			bounding_box = nullptr;
		}
		if (bounding_sphere != nullptr) {
			delete bounding_sphere;
			bounding_sphere = nullptr;
		}
	}
};

// protected 상속 => 외부에서 Model 기능 접근 불가
class BoxCollider : protected Model {
	GLfloat width, height, depth;
public:
	BoxCollider(Model* origin, const GLfloat& w, const GLfloat& h, const GLfloat& d);
	void Render() override;
	// 박스 충돌 체크는 SAT 알고리즘 사용 예정
};

class SphereCollider : protected Model {
	GLfloat radius;
public:
	SphereCollider(Model* origin, const GLfloat& radius);
	void Render() override;
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