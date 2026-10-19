#pragma once
#include "pch.h"
#include "Shader.h"
#include "Texture.h"

// 여러 Object가 공유할 수 있습니다. 개별 색상이 필요하면 Material을 따로 만듭니다.
class Material
{
public:
	//[윤지훈] explicit: 이 클래스가 어떤 함수 인자로 사용되었을 때, 
	// 해당 인자값으로 아래 생성자 둘 중 하나의 인자값만 넣었을 때
	// 암시적 형변환이 일어나 매쉬 객체가 임시 생성되어 전달될 수 있음.
	// 그걸 막아주는 키워드
	explicit Material(std::shared_ptr<Shader> shaderProgram) : shader(std::move(shaderProgram)) {}

	//[윤지훈] 렌더링 직전 이 머터리얼에 관한 렌더 옵션 설정해주는 함수
	void Apply() const;

	//[윤지훈] 사용할 셰이더, 텍스처, 색상
	std::shared_ptr<Shader>	 shader;
	std::shared_ptr<Texture> texture;
	glm::vec3				 color{1.0f};
	//[윤지훈] 퐁 모델 스페큘러(반짝임) 계수
	float					 shininess = 32.0f;
	bool					 useTexture = true;
	bool					 unlit = false;
	//[윤지훈] 환경광만 계산하는지 여부
	bool					 ambientOnly = false;
	//[윤지훈] 양면 설정 시 뒷면 컬링 꺼짐
	bool					 doubleSided = false;
};
