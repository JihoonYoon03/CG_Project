#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tools.h"

bool debug_mode_collider = false;

Model::Model(const std::string& filename, const glm::vec3& size, const glm::vec3& defColor, const CollideMode& collider, const std::string& texture) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit(EXIT_FAILURE);
	}

	std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempTexCoords;
    std::vector<glm::vec3> tempNormals;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            tempVertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            tempTexCoords.push_back(texCoord);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            tempNormals.push_back(normal);
        }
        else if (prefix == "f") {
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;
            auto parse_face = [](const std::string& s) {
                size_t p1 = s.find('/');
                size_t p2 = s.find('/', p1 + 1);
                int vi = std::stoi(s.substr(0, p1)) - 1;
                int ti = std::stoi(s.substr(p1 + 1, p2 - p1 - 1)) - 1;
                int ni = std::stoi(s.substr(p2 + 1)) - 1;
                return std::make_tuple(vi, ti, ni);
            };
            int vi[3], ti[3], ni[3];
            std::tie(vi[0], ti[0], ni[0]) = parse_face(v1);
            std::tie(vi[1], ti[1], ni[1]) = parse_face(v2);
            std::tie(vi[2], ti[2], ni[2]) = parse_face(v3);

            for (int i = 0; i < 3; ++i) {
                renderVertices.push_back(tempVertices[vi[i]]);
                renderTexCoords.push_back(tempTexCoords[ti[i]]);
                renderNormals.push_back(tempNormals[ni[i]]);
            }
        }
    }
	file.close();

	std::cout << "Loaded model: " << filename << " ("
		<< renderVertices.size() << " vertices, "
		<< renderTexCoords.size() << " texcoords, "
		<< renderNormals.size() << " normals)" << std::endl;


	// 센터 구하기
	glm::vec3 min_pos(FLT_MAX), max_pos(-FLT_MAX);
	for (const auto& vertex : renderVertices) {
		min_pos = glm::min(min_pos, vertex);
		max_pos = glm::max(max_pos, vertex);
	}

	center = (min_pos + max_pos) * 0.5f;
	float epsilon = 1e-6f;
	if (std::abs(center.x) < epsilon) center.x = 0.0f;
	if (std::abs(center.y) < epsilon) center.y = 0.0f;
	if (std::abs(center.z) < epsilon) center.z = 0.0f;

	color = new std::vector<glm::vec3>(renderVertices.size(), defColor);

	// 모델 크기 조정
	defScaleMatrix = glm::scale(defScaleMatrix, size);

	glm::vec3 WHD = max_pos - min_pos;
	WHD *= size; // 크기 조정 반영
	GLfloat radius = std::max({ WHD.x, WHD.y, WHD.z }) * 0.5f;
	if (collider & BOX)
		bounding_box = new BoxCollider(this, WHD.x, WHD.y, WHD.z);
	if (collider & SPHERE)
		bounding_sphere = new SphereCollider(this, radius);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VERTEX);
	glGenBuffers(1, &COLOR);
	glGenBuffers(1, &NORMAL);
	glGenBuffers(1, &TEXCOORD);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VERTEX);
	glBufferData(GL_ARRAY_BUFFER, renderVertices.size() * sizeof(glm::vec3), renderVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, COLOR);
	glBufferData(GL_ARRAY_BUFFER, color->size() * sizeof(glm::vec3), color->data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, NORMAL);
	glBufferData(GL_ARRAY_BUFFER, renderNormals.size() * sizeof(glm::vec3), renderNormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, TEXCOORD);
	glBufferData(GL_ARRAY_BUFFER, renderTexCoords.size() * sizeof(glm::vec2), renderTexCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);

	if (texture != "") TEXTURE_ID = loadTexture(texture);
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

void Model::translate(const glm::vec3& dt, const glm::vec3& offset) {
	if (!enabled) return;
	transformQueue.push(glm::translate(glm::mat4(1.0f), -offset));
	transformQueue.push(glm::translate(glm::mat4(1.0f), dt));
}

void Model::Render() {
	if (!enabled) return;
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, renderVertices.size());
	if (debug_mode_collider) {
		if (bounding_box != nullptr) bounding_box->Render();
		if (bounding_sphere != nullptr) bounding_sphere->Render();
	}
}

glm::vec3 Model::retDistTo(const glm::vec3& origin)  {
	glm::vec4 worldLocation = getModelMatrix() * glm::vec4(center, 1.0f);
	return glm::vec3(worldLocation) - origin;
}

glm::mat4 Model::getModelMatrix() {
	while (!transformQueue.empty()) {
		modelMatrix = transformQueue.front() * modelMatrix;
		transformQueue.pop();
	}
	
	glm::mat4 parent_matrix(1.0f);
	if (parent != nullptr) {
		parent_matrix = parent->retParentMatrix();
	}

	return parent_matrix * modelMatrix * defScaleMatrix;
}

// 모델 기본 크기 설정인 defScaleMatrix를 제외하고 반환
glm::mat4 Model::retParentMatrix() {
	while (!transformQueue.empty()) {
		modelMatrix = transformQueue.front() * modelMatrix;
		transformQueue.pop();
	}

	glm::mat4 parent_matrix(1.0f);
	if (parent != nullptr) {
		parent_matrix = parent->retParentMatrix();
	}

	return parent_matrix * modelMatrix;
}

void Model::resetModelMatrix() {
	modelMatrix = glm::mat4(1.0f);
	for (int i = 0; !transformQueue.empty(); i++) {
		transformQueue.pop();
	}
}



BoxCollider::BoxCollider(Model* origin, const GLfloat& w, const GLfloat& h, const GLfloat& d) :
	Model("models/Cube.obj", glm::vec3(w, h, d), glm::vec3(1.0f, 0.0f, 0.0f)), width(w), height(h), depth(d) {
	setParent(origin);
}

void BoxCollider::Render() {
	if (!enabled) return;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


SphereCollider::SphereCollider(Model* origin, const GLfloat& radius) :
	Model("models/Sphere.obj", glm::vec3(radius, radius, radius), glm::vec3(1.0f, 0.0f, 0.0f)), radius(radius) {
	setParent(origin);
}

void SphereCollider::Render() {
	if (!enabled) return;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Ray::Ray(const glm::vec3& ori, const glm::vec3& dir) : origin(ori), direction(glm::normalize(dir)) {}

Ray& Ray::operator=(const Ray& other) {
	if (this == &other) return *this;
	origin = other.origin;
	direction = other.direction;
	return *this;
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

GLuint loadTexture(const std::string& filename) {
	// stbi_load 함수를 사용하여 이미지 파일 로드
	stbi_set_flip_vertically_on_load(true); // OpenGL 텍스처 좌표계에 맞게 이미지 뒤집기
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	if (!data) {
		std::cerr << "Failed to load texture: " << filename << std::endl;
		return 0;
	}
	std::cout << "Texture loaded: " << filename << " ("
		<< width << "x" << height << ", " << channels << " channels)" << std::endl;

	GLenum format = GL_RGB;
	if (channels == 4) format = GL_RGBA;

	// 텍스처 생성 및 설정
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S 축 반복
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T 축 반복
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 축소 필터링
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // 확대 필터링

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data); // 이미지 데이터 메모리 해제

	return textureID; // 텍스처 ID 반환
}