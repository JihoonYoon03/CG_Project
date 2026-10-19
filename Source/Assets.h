#pragma once
#include "pch.h"
#include "Mesh.h"
#include "Texture.h"

//[윤지훈] 같은 에셋 중복 로드를 방지하기 위한 클래스임.
// 언오더드 맵으로 이름:데이터 관계로 에셋 데이터를 가지고 있음
// Scene가 소유하는 작은 캐시입니다. 같은 파일은 한 번만 로드합니다.
class Assets
{
public:
	//[윤지훈] 파일 경로를 통한 에셋 로드 후, 해당 에셋에 대한 공유 포인터를 리턴해줌
	std::shared_ptr<Mesh> LoadMesh(const std::string& path);
	std::shared_ptr<Texture> LoadTexture(const std::string& path);

private:
	std::unordered_map<std::string, std::shared_ptr<Mesh>>	  meshes_;
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures_;
};
