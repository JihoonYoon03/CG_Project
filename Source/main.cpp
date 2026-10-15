#include "pch.h"
#include "Framework.h"
#include "ShootingScene.h"

int main(int argc, char** argv)
{
	try
	{
		// Visual Studio와 탐색기 모두 실행 파일 폴더를 기준으로 리소스를 읽습니다.
		//[윤지훈]현재 프로젝트 전체 절대 경로를 executable에 가져옴. wstring으로 수정?
		std::array<wchar_t, 32768> executable{};
		const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));

		if (length == 0 || length >= executable.size())
		{
			throw std::runtime_error("Cannot locate executable");
		}
		
		//[윤지훈]가져온 경로로 현재 프로그램 작업 디렉터리의 절대 경로 변경
		std::filesystem::current_path(std::filesystem::path(executable.data()).parent_path());

		Framework framework;

		//[윤지훈]ShootingScene 생성과 동시에 unique 포인터로서 소유권을 프레임워크에 전달
		return framework.Run(argc, argv, std::make_unique<ShootingScene>());
	}
	catch (const std::exception& error)
	{
		std::cerr << error.what() << '\n';

		return 1;
	}
}
