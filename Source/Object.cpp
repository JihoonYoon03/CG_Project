#include "pch.h"
#include "Object.h"

glm::mat4 Transform::GetMatrix() const
{
	glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
	matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
	matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
	matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));

	return glm::scale(matrix, scale);
}

Object::Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
	: mesh_(std::move(mesh)), material_(std::move(material))
{
}

Box Object::GetWorldBox() const
{
	//[윤지훈] 매쉬가 있으면 GetBounds(), 없으면 고정크기 박스 반환
	const Box local = mesh_ ? mesh_->GetBounds() : Box{glm::vec3(-0.5f), glm::vec3(0.5f)};
	const glm::mat4 matrix = transform.GetMatrix();
	//[윤지훈] 박스는 0,0,0 기준 최저점과 최고점 2개를 이용해 만들어짐
	Box world{glm::vec3(std::numeric_limits<float>::max()), glm::vec3(std::numeric_limits<float>::lowest())};
	// 회전된 메시도 여덟 꼭짓점으로 월드 AABB를 구합니다.

	for (int corner = 0; corner < 8; ++corner)
	{
		//[윤지훈] 비트 연산으로 x는 매번 최고/최저점, y는 2번마다 최고/최저점,
		// z는 4번마다 최고/최저점의 좌표 정보를 만들어질 정점 좌표로 저장함
		const glm::vec3 point((corner & 1) ? local.max.x : local.min.x, (corner & 2) ? local.max.y : local.min.y, (corner & 4) ? local.max.z : local.min.z);
		//[윤지훈] 여기서 박스는 모델 좌표계로 구해졌기 때문에 현재 오브젝트 변환 행렬 적용해서 변환
		const glm::vec3 transformed = glm::vec3(matrix * glm::vec4(point, 1.0f));
		//[윤지훈] 변환 좌표랑 비교, 왜 필요한지 모름
		world.min = glm::min(world.min, transformed);
		world.max = glm::max(world.max, transformed);
	}

	return world;
}

Sphere Object::GetWorldSphere() const
{
	const Box local = mesh_ ? mesh_->GetBounds() : Box{glm::vec3(-0.5f), glm::vec3(0.5f)};
	//[윤지훈] 매쉬 박스 정보를 통해 구체 지름 구하기
	const glm::vec3 size = (local.max - local.min) * glm::abs(transform.scale);
	const float radius = std::max({size.x, size.y, size.z}) * 0.5f;
	//[윤지훈] 구체 센터 좌표 구하기
	const auto center = glm::vec3(transform.GetMatrix() * glm::vec4((local.min + local.max) * 0.5f, 1.0f));

	return {center, radius};
}
