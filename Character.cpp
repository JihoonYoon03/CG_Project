#include "Character.h"
#include "Background.h"

Camera camera;
std::vector<Player*> player;

float PIXEL_PER_METER = (0.1f);
float RUN_SPEED_KMPH = 20.0f; // Km / Hour(여기서 현실적인 속도를 결정) (km / h)
float RUN_SPEED_MPM = (RUN_SPEED_KMPH * 1000.0f / 60.0f); // Meter / Minute
float RUN_SPEED_MPS = (RUN_SPEED_MPM / 60.0f); // Meter / Second
float RUN_SPEED_PPS = (RUN_SPEED_MPS * PIXEL_PER_METER); // 초당 몇 픽셀을 이동할지 결졍(PPS) (이것이 속도가 됨)

Player::Player(glm::vec3 position, float x, float y, float z) : pos(position) {
	// 주인공 전용 버퍼
	auto Make_Buffer = [&]() {
		// 좌표 버퍼
		glGenBuffers(1, &VBO_position); // 버퍼 ID 발급하여 VBO_position에 저장
		// 색 버퍼
		glGenBuffers(1, &VBO_color); // 버퍼 ID 발급하여 VBO_color에 저장

		// VAO 설정 (그리기 전에는 항상 VAO를 바인딩 한 다음에 그리기를 수행해주면 된다.) (VAO가 VBO들을 담고 있기 때문)
		glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO); // 그려질 도형들의 정점들이 저장된 버퍼 정보를 한데 묶은 VAO 바인드

		// 버텍스 셰이더로 값 전달 및 값 읽어들일 유형 설정 (3개씩 읽어야 정점 하나의 좌표, 정점 하나의 색상이 읽어짐)
		GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos"); // Vertex Shader 프로그램에 존재하는 vPos라는 이름의 속성(변수)가 가진 location값을 가져와 pAttribute 변수에 저장한다.
		glBindBuffer(GL_ARRAY_BUFFER, VBO_position); // 정점 위치 버퍼 바인드 (아래 코드에서 바인딩된 버퍼를 대상으로 한 읽어들이기 유형이 전달됨)
		glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0); // pAttribute에서 읽어온 location을 가진 속성 vPos가 데이터를 읽어들일 유형을 지정 (glm::vec3로 정점 정보를 저장했기에, glm::vec3씩 읽어들이도록 해야 함. 그래야 한 번 읽을 때 정점 하나 만큼을 읽어들일 수 있게 됨)
		glEnableVertexAttribArray(pAttribute); // pAttribute가 읽어들인 location을 가진 속성을 활성화 (즉, vPos가 활성화된다.)

		GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor"); // Vertex Shader 프로그램에 존재하는 vColor라는 이름의 속성(변수)가 가진 location값을 가져와 cAttribute 변수에 저장한다.
		glBindBuffer(GL_ARRAY_BUFFER, VBO_color); // 색 버퍼 바인드 (아래 코드에서 바인딩된 버퍼를 대상으로 한 읽어들이기 유형이 전달됨)
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0); // cAttribute에서 읽어온 location을 가진 속성 vColor가 데이터를 읽어들일 유형을 지정
		glEnableVertexAttribArray(cAttribute); // cAttribute가 읽어들인 location을 가진 속성을 활성화 (즉, vColor가 활성화된다.)
		};
	Make_Buffer();
	trans_mat = glm::mat4(1.0f);
	x *= 0.5f;
	y *= 0.5f;
	z *= 0.5f;
	size_x = x;
	size_y = y;
	size_z = z;
	glm::vec3 P[8] = {
		// 앞면 좌상, 우상, 좌하, 우하
		glm::vec3(-x, +y, +z), // 0
		glm::vec3(+x, +y, +z), // 1
		glm::vec3(-x, -y, +z), // 2
		glm::vec3(+x, -y, +z), // 3
		// 앞에서 봤을 때의 뒷면 좌상, 우상, 좌하, 우하
		glm::vec3(-x, +y, -z), // 4
		glm::vec3(+x, +y, -z), // 5
		glm::vec3(-x, -y, -z), // 6
		glm::vec3(+x, -y, -z)  // 7
	};
	std::uniform_real_distribution<float> urd(0.0f, 1.0f);
	auto add_triangle = [&](int one, int two, int three) {
		v.emplace_back(P[one]);
		v.emplace_back(P[two]);
		v.emplace_back(P[three]);
		auto change_rgb = [](const glm::vec3& p) {
			// 좌표값 p를 단위 벡터로 바꾸어 각 성분이 -1.0f ~ 1.0f의 범위가 되도록 설정하고, 이를 n에 저장
			glm::vec3 n = glm::normalize(p);
			// 0.5f에다가 -1.0f ~ 1.0f의 범위를 가진 n을 절반으로 나눈 -0.5f ~ 0.5f 더해주게 되면, 해당 값은 0.0f ~ 1.0f의 값을 가지게 되어 색상을 나타내기 좋은 값이 된다.
			return glm::vec3(0.5f) + n * 0.5f;
			};
		c.emplace_back(change_rgb(P[one]));
		c.emplace_back(change_rgb(P[two]));
		c.emplace_back(change_rgb(P[three]));
		};
	// 앞면
	add_triangle(0, 2, 3);
	add_triangle(0, 3, 1);

	// 뒷면
	add_triangle(4, 7, 6);
	add_triangle(5, 7, 4);

	// 좌측면
	add_triangle(4, 6, 2);
	add_triangle(4, 2, 0);
	// 우측면
	add_triangle(1, 3, 7);
	add_triangle(1, 7, 5);
	// 윗면
	add_triangle(4, 0, 1);
	add_triangle(4, 1, 5);
	// 밑면
	add_triangle(2, 6, 7);
	add_triangle(2, 7, 3);
}

void Player::Update_Buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(glm::vec3), v.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, c.size() * sizeof(glm::vec3), c.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장
}
void Player::draw_shape() {
	glBindVertexArray(VAO); // 그려질 도형들의 정점 정보가 저장된 VAO 바인드
	// 셰이더 프로그램에서 model_Transform 변수 위치 model_Location으로 가져오기 (한 번만 가져오고, 각 도형에 대해서 행렬 최신화 할거라 상관 없음)
	unsigned int model_Location = glGetUniformLocation(shaderProgramID, "model_Transform");
	// 현재 존재하는 도형 모두 그리기
	int index = 0; // index 1당 정점 하나
	int count = 0; // 해당 오브젝트의 정점 개수 세기
	glm::mat4 T(1.0f);
	T = glm::translate(T, pos);
	//glUniformMatrix4fv(model_Location, 1, GL_FALSE, glm::value_ptr(T * side_rotation * up_rotation * trans_mat));
	glUniformMatrix4fv(model_Location, 1, GL_FALSE, glm::value_ptr(T * side_rotation * trans_mat));

	count = 0; // 정점 개수 초기화
	for (auto vt = v.begin(); vt != v.end(); ++vt) {
		count++; // 정점의 개수 세기
	}
	count /= 3; // 삼각형 개수 세기
	// 삼각형의 개수만큼 반복
	for (int i = 0; i < count; i++) {
		glDrawArrays(GL_TRIANGLES, index, 3);
		index += 3;
	}
}
// 이동
void Player::up_move() {
	glm::vec4 R(1.0f);
	R = side_rotation * glm::vec4(glm::vec3(0.0f, 0.0f, -speed * frame_time), 1.0f);
	pos += glm::vec3(R);
	if (outside_map() or collision()) pos -= glm::vec3(R);
}
void Player::down_move() {
	glm::vec4 R(1.0f);
	R = side_rotation * glm::vec4(glm::vec3(0.0f, 0.0f, speed * frame_time), 1.0f);
	pos += glm::vec3(R);
	if (outside_map() or collision()) pos -= glm::vec3(R);
}
void Player::left_move() {
	glm::vec4 R(1.0f);
	R = side_rotation * glm::vec4(glm::vec3(-speed * frame_time, 0.0f, 0.0f), 1.0f);
	pos += glm::vec3(R);
	if (outside_map() or collision()) pos -= glm::vec3(R);
}
void Player::right_move() {
	glm::vec4 R(1.0f);
	R = side_rotation * glm::vec4(glm::vec3(speed * frame_time, 0.0f, 0.0f), 1.0f);
	pos += glm::vec3(R);
	if (outside_map() or collision()) pos -= glm::vec3(R);
}
// 카메라 위치 세팅
void Player::camera_pos_setting() {
	// 1. 캐릭터의 전방을 side_rotation으로 돌려서 실제 전방을 구함 (어차피 방향만 주면 되기에 정면은 그냥 glm::vec3 baseFront(0.0f, 0.0f, -1.0f)로 처리)
	glm::vec3 baseFront(0.0f, 0.0f, -1.0f);
	glm::vec3 forward = glm::normalize(glm::vec3(up_rotation * side_rotation * glm::vec4(baseFront, 1.0f))); // 정면을 side_rotation만큼 회전 시킨 후 정규화 (안정성을 위한 정규화)
	// 캐릭터 중심점 (카메라 위치 및 회전 시 몸체가 보이지 않도록 하는 높이를 고려해 조금 위쪽으로 설정)
	glm::vec3 center = glm::vec3(pos.x, pos.y + 0.25f, pos.z);
	// 캐릭터로부터 카메라를 둘 위치(머리)
	camera.set_camera_Pos(center + glm::vec3(0.0f, 0.0f, -0.125f));
	// 카메라가 정면을 바라봄
	camera.set_camera_Direction(center + forward);
}
// 회전량 받아와서 저장
void Player::rotation(glm::mat4 side, glm::mat4 up) {
	side_rotation = side;
	up_rotation = up;
}
// 히트 박스 (좌, 우, 앞, 뒤)
glm::vec4 Player::return_hitbox() {
	return glm::vec4(pos.x - size_x, pos.x + size_x, pos.z - size_z, pos.z + size_z);
}
bool Player::outside_map() {
	if (return_hitbox()[0] >= objects[0]->return_hitbox()[1]) return true;
	if (return_hitbox()[1] <= objects[0]->return_hitbox()[0]) return true;
	if (return_hitbox()[2] >= objects[0]->return_hitbox()[3]) return true;
	if (return_hitbox()[3] <= objects[0]->return_hitbox()[2]) return true;
	return false;
}
bool Player::collision() {
	for (size_t i = 1; i < objects.size(); ++i) {
		if (return_hitbox()[0] <= objects[i]->return_hitbox()[1]
			and return_hitbox()[1] >= objects[i]->return_hitbox()[0]
			and return_hitbox()[2] <= objects[i]->return_hitbox()[3]
			and return_hitbox()[3] >= objects[i]->return_hitbox()[2]) return true;
	}
	return false;
}