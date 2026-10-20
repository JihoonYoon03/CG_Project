#include "pch.h"
#include "Texture.h"
//[윤지훈] 각종 이미지 포맷 읽을 수 있게 해주는 라이브러리
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& filename)
{
	//[윤지훈] 이미지의 y축 방향과 opengl y축 방향이 정반대이므로 사전에 뒤집어준다
	stbi_set_flip_vertically_on_load(true);
	int width = 0, height = 0, channels = 0;
	//[윤지훈] 순서대로 파일명, 너비, 높이, 채널 수(R, G, B, A), 요청할 채널 개수(RGBA = 4)
	// 동적 할당된 이미지의 메모리 포인터 반환
	unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error("Cannot load texture: " + filename);
	}

	//[윤지훈] 텍스처 1개 생성 후 ID 저장
	glGenTextures(1, &id_);
	//[윤지훈] 텍스처 타겟에 텍스처 객체 바인딩(연결)
	glBindTexture(GL_TEXTURE_2D, id_);
	//[윤지훈] 타겟, 밉맵레벨(기본0), 텍스처 포맷(RGBA 8비트씩), 너비, 높이, 0 고정, 입력되는 픽셀 데이터 포맷, 입력되는 픽셀 데이터 자료형, 데이터
	//데이터에 NULL 전달 시 메모리만 할당 가능
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	//[윤지훈] 현재 바인딩 된 텍스처 타겟에 대한 밉맵 자동생성
	glGenerateMipmap(GL_TEXTURE_2D);
	//[윤지훈] 현재 바인딩 된 텍스처 타겟에 대한 파라미터 설정
	//순서대로 가로 방향에서 uv를 벗어나는 것에 대한 텍스처 처리 방법 설정, 세로 방향에 대한 ''
	//화면상의 픽셀과 텍셀이 맞지 않을 때, 텍스처 > 그려야 할 픽셀 영역일 때와 그 반대일 때 어떻게 보간할지
	//보간 방법은 여러가지 있으므로 추후 수정 가능
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//[윤지훈] 바인딩 해제하고 사용한 동적 메모리 해제
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pixels);
}

Texture::~Texture()
{
	if (id_)
	{
		glDeleteTextures(1, &id_);
	}
}

//[윤지훈] 텍스처 유닛 활성화 후 ID를 통해 바인딩
void Texture::Bind() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id_);
}
