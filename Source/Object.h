#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Material.h"

//[윤지훈] 변환 행렬이 아니라 벡터 구조체로 만들어서 요소별 파악이 쉬워짐
//GetMatrix에서 연산 후 행렬 리턴. 회전은 현재 오일러 각 사용하므로 쿼터니언 개선 필요
struct Transform
{
	glm::vec3 position{0.0f};
	glm::vec3 rotation{0.0f}; // 도 단위: X(pitch), Y(yaw), Z(roll)
	glm::vec3 scale{1.0f};	  // 각 축은 0이 아닌 값을 사용합니다.
	glm::mat4 GetMatrix() const;
};

// Scene가 Object를 소유하고, Object는 Mesh와 Material을 공유합니다.
class Object
{
public:
	//[윤지훈] 매쉬/머터리얼 미지정 시 이후 Set할 수 있게 해주는 함수 필요
	Object(std::shared_ptr<Mesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);
	virtual ~Object() = default;
	//[윤지훈] 복사 생성자/복사 대입연산자 사용 X
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;

	//[윤지훈] ??
	virtual void Update(float deltaTime) { (void)deltaTime; }
	//[윤지훈] ID는 Scene에서 AddObject 호출 시 정해짐. 즉, 씬에서의 ID임
	uint32_t GetId() const { return id_; }
	const std::shared_ptr<Mesh>& GetMesh() const { return mesh_; }
	const std::shared_ptr<Material>& GetMaterial() const { return material_; }
	//[윤지훈] 자식클래스가 이 함수 오버라이드 후 월드 변환행렬 계산 방식을 커스텀 가능
	//총 클래스에 적용되어있음
	virtual glm::mat4 GetModelMatrix() const { return transform.GetMatrix(); }

	//[윤지훈] 콜라이더 정보 반환
	Box GetWorldBox() const;
	Sphere GetWorldSphere() const;

	std::string	 name;
	Transform	 transform;
	ColliderType collider = ColliderType::None;
	bool		 active = true;	 // false이면 업데이트, 렌더링, 충돌 모두 제외
	bool		 visible = true; // 로컬 플레이어처럼 업데이트만 할 객체에 사용
	bool		 blocksMovement = false;

private:
	friend class Scene;
	uint32_t				  id_ = 0; // Scene 내부 식별자. 네트워크 ID 매핑은 서버 계층에서 관리합니다.
	std::shared_ptr<Mesh>	  mesh_;
	std::shared_ptr<Material> material_;
};
