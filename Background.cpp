#include "Background.h"

extern std::vector<Objects*> objects;

Background::Background(glm::vec3 position, float x, float y, float z) : pos(position) {
	size_x = x;
	size_y = y;
	size_z = z;
	// 객체 전용 버퍼
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
	glm::vec3 P[4] = {
		glm::vec3(-x, 0.0f, -z),
		glm::vec3(+x, 0.0f, -z),
		glm::vec3(-x, 0.0f, +z),
		glm::vec3(+x, 0.0f, +z),
	};
	auto add_triangle = [&](int i, int j, int k) {
		v.emplace_back(P[i]);
		v.emplace_back(P[j]);
		v.emplace_back(P[k]);

		// 초록색 바닥
		c.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
		c.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
		c.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f));
		};
	// 바닥 생성
	add_triangle(0, 2, 3);
	add_triangle(0, 3, 1);
}
void Background::Update_Buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(glm::vec3), v.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, c.size() * sizeof(glm::vec3), c.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장
}
void Background::draw_shape() {
	glBindVertexArray(VAO); // 그려질 도형들의 정점 정보가 저장된 VAO 바인드
	// 셰이더 프로그램에서 model_Transform 변수 위치 model_Location으로 가져오기 (한 번만 가져오고, 각 도형에 대해서 행렬 최신화 할거라 상관 없음)
	unsigned int model_Location = glGetUniformLocation(shaderProgramID, "model_Transform");
	// 현재 존재하는 도형 모두 그리기
	int index = 0; // index 1당 정점 하나
	int count = 0; // 해당 오브젝트의 정점 개수 세기
	glm::mat4 T(1.0f);
	glUniformMatrix4fv(model_Location, 1, GL_FALSE, glm::value_ptr(T));

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
glm::vec4 Background::return_hitbox() {
	// 좌, 우, 앞, 뒤
	return glm::vec4(pos.x - size_x, pos.x + size_x, pos.z - size_z, pos.z + size_z);
}

Box::Box(glm::vec3 position, float x, float y, float z) : pos(position) {
	size_x = x;
	size_y = y;
	size_z = z;
	// 객체 전용 버퍼
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
	auto add_triangle = [&](int one, int two, int three) {
		v.emplace_back(P[one]);
		v.emplace_back(P[two]);
		v.emplace_back(P[three]);

		glm::vec3 color = glm::vec3(0.647f, 0.165f, 0.165f);
		std::uniform_real_distribution<float> urd(0.01f, 0.2f); // 색 흔들어서 입체감 추가
		c.emplace_back(color + glm::vec3(urd(dre), urd(dre), urd(dre)));
		c.emplace_back(color + glm::vec3(urd(dre), urd(dre), urd(dre)));
		c.emplace_back(color + glm::vec3(urd(dre), urd(dre), urd(dre)));
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
void Box::Update_Buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(glm::vec3), v.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장

	glBindBuffer(GL_ARRAY_BUFFER, VBO_color); // 정점 버퍼로 바인딩 (아래 코드에서 바인딩된 버퍼로 데이터가 전달됨)
	glBufferData(GL_ARRAY_BUFFER, c.size() * sizeof(glm::vec3), c.data(), GL_STATIC_DRAW); // 해당 버퍼에 소스 파일에서 선언한 정점 속성 배열 데이터 저장
}
void Box::draw_shape() {
	glBindVertexArray(VAO); // 그려질 도형들의 정점 정보가 저장된 VAO 바인드
	// 셰이더 프로그램에서 model_Transform 변수 위치 model_Location으로 가져오기 (한 번만 가져오고, 각 도형에 대해서 행렬 최신화 할거라 상관 없음)
	unsigned int model_Location = glGetUniformLocation(shaderProgramID, "model_Transform");
	// 현재 존재하는 도형 모두 그리기
	int index = 0; // index 1당 정점 하나
	int count = 0; // 해당 오브젝트의 정점 개수 세기
	glm::mat4 T(1.0f);
	T = glm::translate(T, pos);
	glUniformMatrix4fv(model_Location, 1, GL_FALSE, glm::value_ptr(T));

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
glm::vec4 Box::return_hitbox() {
	// 좌, 우, 앞, 뒤
	return glm::vec4(pos.x - size_x, pos.x + size_x, pos.z - size_z, pos.z + size_z);
}