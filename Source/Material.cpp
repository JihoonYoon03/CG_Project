#include "pch.h"
#include "Material.h"

void Material::Apply() const
{
	//[윤지훈] 프래그먼트 셰이더에서 사용되는 렌더 옵션들 설정 
	shader->Use();
	shader->Set("materialColor", color);
	shader->Set("shininess", shininess);
	shader->Set("unlit", static_cast<int>(unlit));
	shader->Set("ambientOnly", static_cast<int>(ambientOnly));
	shader->Set("useTexture", static_cast<int>(useTexture && texture != nullptr));
	//[윤지훈] 0번 텍스처 유닛(=텍스처 슬롯) 사용
	shader->Set("colorTexture", 0);

	if (texture)
	{
		//[윤지훈] 텍스처 바인딩
		texture->Bind();
	}
	else
	{
		//[윤지훈] 텍스처가 없다면 기본 0번 텍스처 유닛으로 설정 복구
		glActiveTexture(GL_TEXTURE0);
		//[윤지훈] 2번째 인자는 glGenTextures 함수 실행 시 리턴된 텍스처 ID 정수값임.
		// 0 = 바인딩 해제 및 기본으로 되돌리기
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}
}
