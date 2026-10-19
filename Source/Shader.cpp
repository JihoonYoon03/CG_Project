#include "pch.h"
#include "Shader.h"

namespace
{
	//[윤지훈] 파일 경로로 찾은 셰이더 컴파일 및 저장해주는 함수
	//인자는 셰이더 종류/파일 경로
	GLuint Compile(GLenum type, const std::string& filename)
	{
		//[윤지훈] 파일 오픈. 바이너리로 변경해도 될 듯?
		std::ifstream file(filename);

		if (!file)
		{
			throw std::runtime_error("Cannot open shader: " + filename);
		}

		//[윤지훈] stringstream: 문자열 기반 입출력 클래스. iostream이나 ifstream/ofstream같은 거
		std::stringstream buffer;
		//[윤지훈] rdbuf로 버퍼 포인터 반환하고 buffer에 입력시킴
		buffer << file.rdbuf();
		//[윤지훈] std::string에 버퍼 내용을 문자열로 저장
		const std::string text = buffer.str();
		//[윤지훈] char*로 문자열 시작 저장
		const char* source = text.c_str();
		//[윤지훈] type에 맞는 셰이더 객체 생성하고 ID 리턴받기
		const GLuint shader = glCreateShader(type);
		//[윤지훈] 순서대로 셰이더 ID/문자열 개수(=소스코드 조각 수)/문자열 배열/문자열 길이(null시 문자열이 null로 끝난다 보고 자동 계산)
		glShaderSource(shader, 1, &source, nullptr);
		//[윤지훈] 셰이더를 컴파일
		glCompileShader(shader);
		GLint success = GL_FALSE;
		//[윤지훈] 셰이더 컴파일 성공 여부 확인
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		//[윤지훈] 실패 시, 로그 출력 후 종료
		if (!success)
		{
			char log[4096]{};
			glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
			glDeleteShader(shader);
			throw std::runtime_error(filename + ": " + log);
		}

		return shader;
	}
} // namespace

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
	const GLuint vertex = Compile(GL_VERTEX_SHADER, vertexPath);
	GLuint fragment = 0;

	try
	{
		fragment = Compile(GL_FRAGMENT_SHADER, fragmentPath);
	}
	catch (...)
	{
		glDeleteShader(vertex);
		throw;
	}

	//[윤지훈] 현재 프로그램 객체에 셰이더 연결 및 각종 세팅
	program_ = glCreateProgram();
	glAttachShader(program_, vertex);
	glAttachShader(program_, fragment);
	glLinkProgram(program_);
	//[윤지훈] 연결 끝나면 삭제해도 됨
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	GLint success = GL_FALSE;
	//[윤지훈] 성공 여부 조사
	glGetProgramiv(program_, GL_LINK_STATUS, &success);

	if (!success)
	{
		char log[4096]{};
		glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
		glDeleteProgram(program_);
		program_ = 0;
		throw std::runtime_error(std::string("Shader link failed: ") + log);
	}
}

Shader::~Shader()
{
	if (program_)
	{
		glDeleteProgram(program_);
	}
}

void Shader::Use() const
{
	glUseProgram(program_);
}

//[윤지훈] Uniform 변수의 정수 위치값을 이름으로 찾아주는 함수
GLint Shader::Location(const std::string& name) const
{
	const auto found = locations_.find(name);

	//[윤지훈] 맵 컨테이너에서 찾은 위치의 정수값 리턴
	if (found != locations_.end()) return found->second;

	//[윤지훈] 맵 컨테이너에 없는 경우, name에 해당하는 정수값을 얻어와 저장한다.
	const GLint location = glGetUniformLocation(program_, name.c_str());
	locations_[name] = location;

	return location;
}

void Shader::Set(const std::string& name, int value) const
{
	glUniform1i(Location(name), value);
}

void Shader::Set(const std::string& name, float value) const
{
	glUniform1f(Location(name), value);
}

void Shader::Set(const std::string& name, const glm::vec3& value) const
{
	glUniform3fv(Location(name), 1, glm::value_ptr(value));
}

void Shader::Set(const std::string& name, const glm::mat4& value) const
{
	glUniformMatrix4fv(Location(name), 1, GL_FALSE, glm::value_ptr(value));
}
