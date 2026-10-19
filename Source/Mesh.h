#pragma once
#include "pch.h"
#include "Collision.h"

//[윤지훈] 현재 정점 데이터는 위치, 노멀, uv좌표 데이터만 갖고있다.
struct Vertex
{
	glm::vec3 position{0.0f};
	glm::vec3 normal{0.0f, 1.0f, 0.0f};
	glm::vec2 texCoord{0.0f};
};

// 정점과 GPU 버퍼를 소유합니다. 위치, 색, 게임 규칙은 저장하지 않습니다.
class Mesh
{
public:
	//[윤지훈] explicit: 이 클래스가 어떤 함수 인자로 사용되었을 때, 
	// 해당 인자값으로 아래 생성자 둘 중 하나의 인자값만 넣었을 때
	// 암시적 형변환이 일어나 매쉬 객체가 임시 생성되어 전달될 수 있음.
	// 그걸 막아주는 키워드
	explicit Mesh(const std::string& filename);
	explicit Mesh(std::vector<Vertex> vertices);
	~Mesh();
	//[윤지훈] 복사생성, 복사대입 생성 차단
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	void Draw() const;

	//[윤지훈] 정점데이터 리턴
	const std::vector<Vertex>& GetVertices() const { return vertices_; }
	//[윤지훈] const Box&를 리턴하는 데이터 변경 없는(const) 함수
	const Box& GetBounds() const { return bounds_; }

private:
	void Upload();

	std::vector<Vertex> vertices_;
	Box					bounds_;
	//[윤지훈] VAO(Vertex Array Object)는 정점 버퍼를 어떻게 읽는지에 대한 설명서와 같음.
	GLuint				vao_ = 0;
	//[윤지훈] VBO(Vertex Buffer Object)는 실제 정점 데이터를 저장하고 있는 버퍼 오브젝트.
	GLuint				vbo_ = 0;
};
