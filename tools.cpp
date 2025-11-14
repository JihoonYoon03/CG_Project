#include "tools.h"

Model::Model(const std::string& filename, const glm::vec3& size, const glm::vec3& defColor) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;

		if (prefix == "v") {
			glm::vec3 vertex;
			iss >> vertex.x >> vertex.y >> vertex.z;
			vertices.push_back(vertex);
		}
		else if (prefix == "f") {
			std::string v1, v2, v3;
			iss >> v1 >> v2 >> v3;

			// "정점/텍스처/노멀"에서 정점 인덱스만 추출
			glm::uvec3 face;
			face.x = std::stoi(v1.substr(0, v1.find('/'))) - 1;
			face.y = std::stoi(v2.substr(0, v2.find('/'))) - 1;
			face.z = std::stoi(v3.substr(0, v3.find('/'))) - 1;

			faces.push_back(face);
		}
		else if (prefix == "vn") {
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
	}
	file.close();

	std::cout << "Loaded " << vertices.size() << " vertices, "
		<< faces.size() << " faces" << std::endl;  // 디버그 출력

	// 모델 크기 조정
	for (auto& vertex : vertices) {
		vertex *= size;
	}

	/*for (auto& normal : normals) {
		normal *= size;
		normal = glm::normalize(normal);
	}*/


	glm::vec3 min_pos(FLT_MAX), max_pos(-FLT_MAX);
	for (const auto& vertex : vertices) {
		min_pos = glm::min(min_pos, vertex);
		max_pos = glm::max(max_pos, vertex);
	}

	center = (min_pos + max_pos) * 0.5f;

	color = new std::vector<glm::vec3>(vertices.size(), defColor);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &COLOR);
	glGenBuffers(1, &NORMAL);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
		vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(glm::uvec3),
		faces.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, COLOR);
	glBufferData(GL_ARRAY_BUFFER, color->size() * sizeof(glm::vec3), color->data(), GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, NORMAL);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	basis = new DisplayBasis(0.2f, center);
}

void Model::setParent(Model* parent) {
	if (enabled) this->parent = parent;
}

void Model::setDefScale(const glm::vec3& ds) {
	if (enabled) default_scale = glm::scale(glm::mat4(1.0f), ds);
}
void Model::setDefRotate(const glm::mat4& dr) {
	if (enabled) default_rotate = dr;
}
void Model::setDefTranslate(const glm::vec3& dt) {
	if (enabled) default_translate = glm::translate(glm::mat4(1.0f), dt);
}

void Model::scale(const glm::vec3& scaleFactor) {
	if (enabled) transformQueue.push(glm::scale(glm::mat4(1.0f), scaleFactor));
}

void Model::rotate(GLfloat angle, const glm::vec3& axis) {
	if (enabled) transformQueue.push(glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis));
}

void Model::translate(const glm::vec3& delta) {
	if (enabled) transformQueue.push(glm::translate(glm::mat4(1.0f), delta));
}

void Model::Render() {
	if (!enabled) return;
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);

	// 주석 해제 시 모델 중심 좌표계 출력
	//basis->Render();
}

glm::vec3 Model::retDistTo(const glm::vec3& origin) {
	glm::vec4 worldLocation = modelMatrix * default_translate * default_rotate * default_scale * glm::vec4(center, 1.0f);
	return glm::vec3(worldLocation) - origin;
}

glm::mat4 Model::getModelMatrix() {
	while (!transformQueue.empty()) {
		modelMatrix = transformQueue.front() * modelMatrix;
		transformQueue.pop();
	}
	
	glm::mat4 parent_matrix(1.0f);
	if (parent != nullptr) {
		parent_matrix = parent->getModelMatrix();
	}

	
	// 모델 중심 변환 -> 모델 변환 -> 부모 변환
	return parent_matrix * modelMatrix * default_translate * default_rotate * default_scale;
}

void Model::resetModelMatrix() {
	modelMatrix = glm::mat4(1.0f);
	for (int i = 0; !transformQueue.empty(); i++) {
		transformQueue.pop();
	}
}

Model::~Model() {
	delete color;
	delete basis;
}

DisplayBasis::DisplayBasis(GLfloat offset, const glm::vec3& origin) : origin(origin) {

	xyz[0][0] = { origin, { 0.1f, 0.1f, 0.0f } };
	xyz[0][1] = { origin + glm::vec3(1.0f, 0.0f, 0.0f), { 1.0f, 0.0f, 0.0f } };
	xyz[1][0] = { origin, { 0.0f, 0.1f, 0.1f } };
	xyz[1][1] = { origin + glm::vec3(0.0f, 1.0f, 0.0f), { 0.0f, 1.0f, 0.0f } };
	xyz[2][0] = { origin, { 0.1f, 0.0f, 0.1f } };
	xyz[2][1] = { origin + glm::vec3(0.0f, 0.0f, 1.0f), { 0.0f, 0.0f, 1.0f } };

	for (int i = 0; i < 3; i++) {
		xyz[i][0].pos *= offset;
		xyz[i][1].pos *= offset;
	}
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(xyz), xyz, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);
}

void DisplayBasis::Render() {
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 6);
}

void mPosToGL(GLuint winWidth, GLuint winHeight, int mx, int my, GLfloat& xGL, GLfloat& yGL)
{
	xGL = (mx / (winWidth / 2.0f)) - 1.0f;
	yGL = 1.0f - (my / (winHeight / 2.0f));
}

std::string read_file(const std::string& filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void make_vertexShaders(GLuint& vertexShader, const std::string& shaderName) {
	std::string vertexSource = read_file(shaderName);
	if (vertexSource.empty()) {
		std::cerr << "ERROR: Failed to read vertex shader file" << std::endl;
		return;
	}

	const char* sourcePtr = vertexSource.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &sourcePtr, nullptr);
	glCompileShader(vertexShader);

	GLint result;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLchar errorLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
	}
}

void make_fragmentShaders(GLuint& fragmentShader, const std::string& shaderName) {
	std::string fragmentSource = read_file(shaderName);
	if (fragmentSource.empty()) {
		std::cerr << "ERROR: Failed to read fragment shader file" << std::endl;
		return;
	}

	const char* sourcePtr = fragmentSource.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &sourcePtr, nullptr);
	glCompileShader(fragmentShader);

	GLint result;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLchar errorLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
	}
}

GLuint make_shaderProgram(const GLuint& vertexShader, const GLuint& fragmentShader) {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	glLinkProgram(shaderID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		GLchar errorLog[512];
		glGetProgramInfoLog(shaderID, 512, nullptr, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return 0;
	}

	glUseProgram(shaderID);
	return shaderID;
}