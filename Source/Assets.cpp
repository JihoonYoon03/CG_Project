#include "pch.h"
#include "Assets.h"

std::shared_ptr<Mesh> Assets::LoadMesh(const std::string& path)
{
	const auto found = meshes_.find(path);

	//[윤지훈] 이미 로드된 매쉬인 경우 바로 반환
	if (found != meshes_.end()) return found->second;

	//[윤지훈] 로드 안 된 매쉬인 경우 새로 만들어서 저장하고 리턴
	auto mesh = std::make_shared<Mesh>(path);
	meshes_[path] = mesh;

	return mesh;
}

//[윤지훈] 위와 로직 동일
std::shared_ptr<Texture> Assets::LoadTexture(const std::string& path)
{
	const auto found = textures_.find(path);

	if (found != textures_.end()) return found->second;

	auto texture = std::make_shared<Texture>(path);
	textures_[path] = texture;

	return texture;
}
