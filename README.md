# 사격 게임 개발 베이스

이 프로젝트는 C++17, OpenGL 3.3, FreeGLUT, GLEW, GLM을 사용합니다.
C++ 소스와 셰이더는 **CP949(한국어 코드 페이지 949)** 로 저장하고,
이 README와 Markdown 문서는 **UTF-8**로 저장합니다.
소스 파일을 Visual Studio에서 열었을 때 한글이 깨지면 한국어(코드 페이지 949)로 다시 엽니다.
README를 열 때는 UTF-8을 사용합니다.
프로젝트의 `/source-charset:.949`와 `/execution-charset:.949` 설정을 유지합니다.

## 빌드와 실행

1. `CG_Project.sln`을 열고 **x64 / Debug 또는 Release**를 선택합니다.
2. 현재 설치 환경의 v145 도구 집합을 사용합니다. 다른 PC에서는 해당 도구 집합을 설치하거나
   팀에서 합의한 설치 버전으로 프로젝트 속성을 변경합니다.
3. 동봉된 `lib/freeglut.lib`, `lib/glew32.lib`는 x64용입니다. Win32 빌드에는 별도의 x86 라이브러리가 필요합니다.
4. 실행 PC에는 대응되는 x64 `freeglut.dll`, `glew32.dll`이 필요합니다. `lib` 폴더에 두면 빌드 시 실행 파일 옆으로 복사합니다.
   현재 저장소에는 DLL이 포함되어 있지 않습니다.
5. 빌드 시 `models`와 `Shaders`도 출력 폴더로 복사합니다. 실행 파일은 자신의 폴더를 기준으로 자원을 읽습니다.

- WASD: 이동 / 마우스: 시점 / 좌클릭: 사격
- 1, 2, 3: 스테이지 시작 / F: 손전등 / /: 충돌체 표시
- Esc: 마우스 잡기/해제 / 창을 다시 클릭: 마우스 잡기 / Q: 종료
- `CG_Project.exe --self-test`: 창 없이 충돌, 입력, 이동, 스테이지 회귀 검사
- `CG_Project.exe --smoke-test [capture.ppm]`: 숨긴 창에서 셰이더/모델/조명 로딩 및 실제 장면의 사격/자원 공유 회귀 검사와 40프레임 렌더링 검사

## 처음 읽을 순서

`main.cpp -> Engine/Framework.cpp -> Game/ShootingScene.cpp -> Engine/Object.h`

| 위치 | 책임 |
| --- | --- |
| pch.h / pch.cpp | 공통 표준/외부 라이브러리 헤더와 PCH 생성 |
| Engine/Framework | 창, 입력 콜백, 고정 시간 업데이트, 모든 렌더링, HUD |
| Engine/Scene | 오브젝트 소유 목록, 카메라, 조명, 공통 업데이트 |
| Engine/Object | 오브젝트 ID, Transform, Mesh/Material 포인터, 충돌 속성 |
| Engine/Mesh | OBJ 로딩, 정점 정보, GPU 버퍼 |
| Engine/Material | 셰이더/텍스처 포인터, 색상, 광택, 렌더링 속성 |
| Engine/Shader, Texture | OpenGL 자원 생성/해제와 오류 처리 |
| Engine/Assets | Scene별 메시/텍스처 공유 캐시 |
| Engine/Collision | 그래픽 API에 의존하지 않는 AABB/광선 충돌 계산 |
| Engine/Light | Directional, Point, Spot 조명 데이터 |
| Game/ShootingScene | 현재 게임의 오브젝트 배치, 입력 해석, 발사, HUD 내용 |
| Game/Player, Gun, Target | 이동/카메라 연결, 반동, 피격 상태 |
| Game/StageController | 대기/준비/진행/실패/완료 상태, 시간, 점수, 생성 주기 |
| Shaders/Lit.vert, Lit.frag | 변환, 텍스처, 세 종류 조명 계산 |
| Tests/SelfTests | 그래픽 창 없이 실행하는 핵심 로직 회귀 검사 |

## 한 프레임의 흐름

1. Framework의 GLUT 콜백이 Input에 키와 마우스 상태를 기록합니다.
2. Framework는 누적 시간을 사용해 초당 120회의 고정 간격으로 Scene를 업데이트합니다.
   창 이동 등 긴 정지 후에는 한 번에 최대 0.25초만 따라잡습니다.
3. ShootingScene는 스테이지 이벤트를 처리하고 Input을 PlayerCommand로 변환합니다.
4. Player는 벽과 충돌하지 않는 위치로 이동한 뒤 카메라 위치를 갱신합니다.
5. 발사가 있으면 가장 가까운 활성 표적 하나에만 피격을 적용합니다.
6. Scene의 활성 Object를 업데이트합니다. Gun 반동도 이 단계에서 진행합니다.
7. Framework가 활성/표시 상태인 모든 Object의 Material을 적용하고 Mesh를 그립니다.
8. 충돌 디버그 선과 HUD를 그린 뒤 화면을 교체합니다.

OpenGL 호출은 메인 스레드에서만 수행합니다. `Draw`/`Render`에서는 게임 시간을 진행시키지 않습니다.

## 소유권 규칙

- Framework는 현재 Scene 하나를 `unique_ptr`로 소유합니다.
- Scene는 `vector<unique_ptr<Object>>`로 오브젝트를 소유합니다.
- Object는 `shared_ptr<Mesh>`와 `shared_ptr<Material>`을 가집니다.
- Material은 Shader와 Texture를 `shared_ptr`로 공유합니다.
- ShootingScene의 player_, gun_, targets_는 Scene 소유 객체를 가리키는 관찰 포인터입니다.
  현재 게임에서는 장면 종료까지 삭제하지 않고 active를 전환합니다.
- 동적 삭제를 추가할 때는 관찰 포인터를 먼저 정리하거나 `GetId()/FindObject()`로 다시 조회합니다.
  업데이트 목록을 순회하는 도중 AddObject/RemoveObject를 호출하지 않습니다.
- GPU 자원은 복사를 금지하고 소멸자에서 해제합니다. Framework는 OpenGL 컨텍스트 삭제 전에 Scene를 해제합니다.

## 콘텐츠 추가 예시

정적인 오브젝트는 ShootingScene::Initialize에서 `AddProp`으로 추가합니다.
움직이거나 상호작용하는 오브젝트는 `Object`를 상속하고 `Update(float)`를 구현한 뒤
`AddObject(std::make_unique<새클래스>(mesh, material))`로 등록합니다.
메시와 텍스처는 `assets_.LoadMesh/LoadTexture`를 사용하면 동일 파일이 공유됩니다.
색을 개별 변경할 오브젝트에는 별도의 Material을 만들어야 합니다.

Transform은 위치, 도 단위 회전, 크기만 저장합니다. 월드 행렬은 T * Ry * Rx * Rz * S입니다.
크기 축에 0을 넣으면 노멀 행렬을 계산할 수 없으므로 사용하지 않습니다.
OBJ는 v/vt/vn, v//vn, 위치만 있는 면, 음수 인덱스와 볼록 다각형을 지원합니다.
오목한 다각형과 MTL은 지원하지 않으므로 모델을 삼각화하고 Material에서 텍스처를 지정합니다.

## 조명 추가

Scene의 lights_에 `Light::Directional`, `Light::Point`, `Light::Spot` 결과를 추가합니다.
Directional은 평행광, Point는 위치에서 모든 방향으로 나가는 빛,
Spot은 방향과 내부/외부 각도가 있는 빛입니다. Point/Spot은 range 안에서 부드럽게 감쇠합니다.
기본 장면에는 태양, 푸른 포인트 조명, 카메라를 따르는 손전등이 있습니다.
동시에 최대 8개를 전달합니다. 상한 변경 시 Light::MaxCount와 Lit.frag의 MAX_LIGHTS를 함께 변경합니다.
새 머터리얼 셰이더는 Lit.vert/Lit.frag의 uniform 이름을 따릅니다. 불필요한 uniform은 생략할 수 있습니다.

## 충돌과 게임 규칙

플레이어 이동은 박스 충돌로 X/Z축을 각각 검사합니다. 높이도 비교하므로 위아래로 분리된 박스는 충돌하지 않습니다.
프레임당 작은 이동을 전제로 합니다. 매우 빠른 물체/순간이동에는 별도 swept 충돌 처리가 필요합니다.
표적은 구 충돌체를 사용합니다. 발사 순간에만 광선을 검사하여 이전 발사가 계속 맞히는 문제를 방지합니다.
현재 사격 게임은 기존 규칙대로 표적만 맞히며 울타리는 총알을 막지 않습니다.
새 장애물이 총알을 막아야 한다면 Box Raycast 거리와 표적 거리 중 더 가까운 쪽을 선택합니다.
StageController의 stages_ 표에서 제한 시간, 목표 점수, 초당 표적 수를 변경할 수 있습니다.

## 3인 개발과 멀티플레이 연결 지점

작업을 Framework/렌더링, Scene/콘텐츠, 네트워크/입력 동기화로 나누면 충돌을 줄일 수 있습니다.
공통 헤더에는 외부 라이브러리만 넣고 게임 헤더는 사용하는 파일에서 직접 포함합니다.
새 cpp는 반드시 첫 줄에 `#include "pch.h"`를 넣고 Visual Studio 프로젝트에 추가합니다.

현재 소켓 통신은 구현하지 않았습니다. 먼저 로컬 동작을 유지하는 확장 지점을 제공합니다.

1. 입력 또는 수신 패킷을 PlayerCommand로 변환해 같은 명령 처리 경로를 사용합니다.
2. 수신 스레드는 데이터를 큐에 넣고 메인 스레드의 Scene::Update 전후에 적용합니다.
   수신 스레드에서 Scene 목록이나 OpenGL 객체를 직접 변경하지 않습니다.
3. Object ID는 현재 Scene 내부 ID입니다. 서버가 발급하는 네트워크 ID와의 매핑을 별도로 둡니다.
4. 패킷은 필드별로 직렬화합니다. shared_ptr, glm 객체, C++ 구조체 메모리를 그대로 전송하지 않습니다.
5. 서버에서 명령 범위, 이동 속도, 발사 빈도와 판정을 검증하고 클라이언트는 결과 상태를 반영합니다.

게임 로직과 OpenGL의 책임은 분리했지만 독립 서버 빌드까지 분리한 상태는 아닙니다.
서버 프로젝트를 만들 때 PlayerCommand, StageController, Collision과 상태 데이터를 공유 모듈로 옮기고
PCH에서 그래픽 헤더를 분리하면 됩니다. HUD와 디버그 선은 GLUT 호환 프로파일을 사용하므로
향후 Core Profile로 전환할 때 Framework의 해당 두 함수도 셰이더 기반으로 변경해야 합니다.

## 코드 정렬

들여쓰기는 공백 4칸, 중괄호는 다음 줄, 긴 줄은 120자를 기준으로 정리합니다.
.clang-format에 공통 규칙을 저장했습니다. C++ 구현 파일의 첫 번째 include는 pch.h를 유지합니다.

- 정렬 적용: python scripts/format_code.py
- 정렬 검사: python scripts/format_code.py --check

스크립트는 Visual Studio에 포함된 clang-format을 자동으로 찾습니다.
다른 위치에 설치했다면 --formatter 옵션으로 실행 파일 경로를 지정합니다.
포맷터에 전달할 때만 UTF-8로 변환하고, 결과 소스는 CP949/CRLF로 저장해 한글 주석을 보존합니다.
외부 라이브러리와 모델 파일은 정렬 대상에서 제외합니다.
