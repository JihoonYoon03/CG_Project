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

	// 센터 구하기
	glm::vec3 min_pos(FLT_MAX), max_pos(-FLT_MAX);
	for (const auto& vertex : vertices) {
		min_pos = glm::min(min_pos, vertex);
		max_pos = glm::max(max_pos, vertex);
	}

	center = (min_pos + max_pos) * 0.5f;
	
	color = new std::vector<glm::vec3>(vertices.size(), defColor);

	// 모델 크기 조정
	this->scale(size, center);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VERTEX);
	glGenBuffers(1, &FACE);
	glGenBuffers(1, &COLOR);
	glGenBuffers(1, &NORMAL);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VERTEX);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
		vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FACE);
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
}

void Model::setParent(Model* parent) {
	if (enabled) this->parent = parent;
}

void Model::scale(const glm::vec3& ds, const glm::vec3& origin) {
	if (!enabled) return;
	glm::vec3 dist = retDistTo(origin);
	transformQueue.push(glm::translate(glm::mat4(1.0f), -dist));
	transformQueue.push(glm::scale(glm::mat4(1.0f), ds));
	transformQueue.push(glm::translate(glm::mat4(1.0f), dist));
}

void Model::rotate(const glm::vec3& dr, const glm::vec3& origin) {
	if (!enabled) return;
	glm::vec3 dist = retDistTo(origin);
	glm::mat4 rotate = glm::mat4(1.0f);
	transformQueue.push(glm::translate(glm::mat4(1.0f), -dist));
	rotate = glm::rotate(rotate, glm::radians(dr.x), glm::vec3(1, 0, 0));
	rotate = glm::rotate(rotate, glm::radians(dr.y), glm::vec3(0, 1, 0));
	rotate = glm::rotate(rotate, glm::radians(dr.z), glm::vec3(0, 0, 1));
	transformQueue.push(rotate);
	transformQueue.push(glm::translate(glm::mat4(1.0f), dist));
}

void Model::translate(const glm::vec3& dt) {
	if (!enabled) return;
	transformQueue.push(glm::translate(glm::mat4(1.0f), dt));
}

void Model::Render() {
	if (!enabled) return;
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
}

glm::vec3 Model::retDistTo(const glm::vec3& origin) {
	glm::vec4 worldLocation = modelMatrix * glm::vec4(center, 1.0f);
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

	return parent_matrix * modelMatrix;
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