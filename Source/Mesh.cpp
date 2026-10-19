#include "pch.h"
#include "Mesh.h"

namespace
{
	int ParseIndex(const std::string& token, size_t count)
	{
		const int index = std::stoi(token);
		const int resolved = index > 0 ? index - 1 : static_cast<int>(count) + index;

		if (index == 0 || resolved < 0 || resolved >= static_cast<int>(count))
		{
			throw std::runtime_error("Invalid OBJ index: " + token);
		}

		return resolved;
	}
} // namespace

Mesh::Mesh(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file)
	{
		throw std::runtime_error("Cannot open mesh: " + filename);
	}

	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec2> texCoords;
	std::string line;

	while (std::getline(file, line))
	{
		std::istringstream input(line);
		std::string type;
		input >> type;

		if (type == "v")
		{
			glm::vec3 value;

			if (!(input >> value.x >> value.y >> value.z))
			{
				throw std::runtime_error("Invalid vertex: " + filename);
			}

			positions.push_back(value);
		}
		else if (type == "vn")
		{
			glm::vec3 value;

			if (!(input >> value.x >> value.y >> value.z))
			{
				throw std::runtime_error("Invalid normal: " + filename);
			}

			normals.push_back(value);
		}
		else if (type == "vt")
		{
			glm::vec2 value;

			if (!(input >> value.x >> value.y))
			{
				throw std::runtime_error("Invalid UV: " + filename);
			}

			texCoords.push_back(value);
		}
		else if (type == "f")
		{
			std::vector<Vertex> polygon;
			bool missingNormal = false;
			std::string token;

			while (input >> token)
			{
				if (token.front() == '#')
				{
					break;
				}

				std::istringstream indices(token);
				std::string position, uv, normal;
				std::getline(indices, position, '/');
				std::getline(indices, uv, '/');
				std::getline(indices, normal, '/');
				Vertex vertex;
				vertex.position = positions.at(ParseIndex(position, positions.size()));

				if (!uv.empty())
				{
					vertex.texCoord = texCoords.at(ParseIndex(uv, texCoords.size()));
				}

				if (!normal.empty())
				{
					vertex.normal = normals.at(ParseIndex(normal, normals.size()));
				}
				else
				{
					missingNormal = true;
				}

				polygon.push_back(vertex);
			}

			if (polygon.size() < 3)
			{
				throw std::runtime_error("Invalid face: " + filename);
			}

			// 삼각형과 볼록 다각형을 지원합니다. 오목한 면은 제작 도구에서 삼각화합니다.

			for (size_t i = 1; i + 1 < polygon.size(); ++i)
			{
				std::array<Vertex, 3> triangle{polygon[0], polygon[i], polygon[i + 1]};

				if (missingNormal)
				{
					const auto cross = glm::cross(triangle[1].position - triangle[0].position, triangle[2].position - triangle[0].position);
					const auto normal = glm::length(cross) > 0.000001f ? glm::normalize(cross) : glm::vec3(0, 1, 0);

					for (auto& vertex : triangle)
					{
						vertex.normal = normal;
					}
				}

				vertices_.insert(vertices_.end(), triangle.begin(), triangle.end());
			}
		}
	}

	if (vertices_.empty())
	{
		throw std::runtime_error("Empty mesh: " + filename);
	}

	Upload();
}

Mesh::Mesh(std::vector<Vertex> vertices) : vertices_(std::move(vertices))
{
	if (vertices_.empty())
	{
		throw std::runtime_error("Empty mesh");
	}

	Upload();
}

void Mesh::Upload()
{
	bounds_.min = bounds_.max = vertices_.front().position;

	for (const auto& vertex : vertices_)
	{
		bounds_.min = glm::min(bounds_.min, vertex.position);
		bounds_.max = glm::max(bounds_.max, vertex.position);
	}

	//[윤지훈] 주어진 개수만큼의 VAO, VBO 버퍼 공간 생성 후 식별 번호를 vao_, vbo_에 저장
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);

	//[윤지훈] vao_, vbo_ 에 저장된 버퍼 공간을 현재 작업할 대상으로 지정시킴
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	//[윤지훈] 정점 데이터 업로드. 순서대로 버퍼종류/전체크기/실제 데이터 위치/데이터 사용 목적 
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);

	//[윤지훈] Lit.vert 셰이더와 연결되는 부분
	//순서대로 Lit.vert에서 사용되는 정점 속성 번호/데이터 개수/타입/정규화 여부/데이터 1개 사이즈/데이터 위치(void*타입)
	//offsetof는 구조체 시작 주소에서 멤버 변수까지의 바이트 거리를 패딩까지 포함해서 정확하게 리턴해줌
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texCoord)));

	//[윤지훈] Lit.vert 셰이더의 layout(location = 0) 형식으로 지정된 정점 속성들을 활성화
	for (GLuint index = 0; index < 3; ++index)
	{
		glEnableVertexAttribArray(index);
	}

	//[윤지훈] vao_ 연결 해제. 안해도 작동할 순 있는데 나중에 예상치 못한 오류 발생하면 답 없음
	glBindVertexArray(0);
}

Mesh::~Mesh()
{
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
	}

	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
	}
}

void Mesh::Draw() const
{
	//[윤지훈] 그리기 전 vao 할당
	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
	glBindVertexArray(0);
}
