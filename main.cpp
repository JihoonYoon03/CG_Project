#include "pch.h"
#include "Engine/Framework.h"
#include "Game/ShootingScene.h"
#include "Tests/SelfTests.h"
#include "Tests/RenderTests.h"

int main(int argc, char** argv)
{
    try
    {
        if (argc > 1 && std::string(argv[1]) == "--self-test")
        {
            return RunSelfTests();
        }
        const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";
        const std::string capture = smokeTest && argc > 2 ? std::filesystem::absolute(argv[2]).string() : "";
        // Visual Studio와 탐색기에서 실행할 때 모두 실행 파일 옆의 리소스를 사용합니다.
        std::array<wchar_t, 32768> executable{};
        const DWORD length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if (length == 0 || length >= executable.size())
        {
            throw std::runtime_error("Cannot locate executable");
        }
        std::filesystem::current_path(std::filesystem::path(executable.data()).parent_path());
        Framework framework;
        std::unique_ptr<Scene> scene;
        if (smokeTest)
        {
            // 캡처 재현용 값입니다. 일반 실행의 콘텐츠나 조작에는 영향을 주지 않습니다.
            scene = CreateRenderTestScene(argc > 3 ? std::stof(argv[3]) : 0, argc > 4 ? std::stof(argv[4]) : 0,
                                          argc > 5 ? std::stof(argv[5]) : -1, argc > 6 ? std::stof(argv[6]) : 0);
        }
        else
        {
            scene = std::make_unique<ShootingScene>();
        }
        return framework.Run(argc, argv, std::move(scene), smokeTest, capture);
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
