#include "basic.h"
std::random_device dre;

// 윈도우 크기
GLint width, height;
//--- 세이더 프로그램 이름
GLuint shaderProgramID;
//--- 버텍스세이더객체
GLuint vertexShader;
//--- 프래그먼트 세이더객체
GLuint fragmentShader;

// 프레임 시간
float frame_time = 0.0f;
auto prev_time = std::chrono::steady_clock::now();

// 마우스 감도
extern float mouse_sensitivity;

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

// 버텍스 셰이더 객체 만들기
void basic_make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("basic_vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
//--- 프래그먼트 세이더 객체 만들기
void basic_make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("basic_fragment.glsl");    // 프래그먼트 세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
// 셰이더 프로그램
GLuint basic_make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	//--- 세이더프로그램만들기
	shaderID = glCreateProgram();
	//--- 세이더프로그램에버텍스세이더붙이기
	glAttachShader(shaderID, vertexShader);
	//--- 세이더프로그램에프래그먼트세이더붙이기
	glAttachShader(shaderID, fragmentShader);
	//--- 세이더프로그램링크하기
	glLinkProgram(shaderID);
	//--- 세이더 객체를 세이더 프로그램에 링크했으므로,세이더 객체 자체는 삭제 가능
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		// ---세이더가 잘연결되었는지체크하기
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	//--- 만들어진 세이더 프로그램 사용하기
	//--- 여러 개의 세이더 프로그램을 만들 수 있고, 그 중 한 개의 프로그램을 사용하려면 glUseProgram 함수를 호출하여 사용할 특정 프로그램을 지정한다.
	//--- 사용하기 직전에 호출할 수 있다.
	glUseProgram(shaderID);
	return shaderID;
}

void Reshape(int w, int h) {
	width = w;
	height = h;
	glViewport(0, 0, w, h); // 전체 화면 영역 설정
}

// 시간 계산
void calculateFrameTime() {
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> dt = now - prev_time;
	frame_time = dt.count();
	prev_time = now;
	glutSetWindowTitle(("FPS: " + std::to_string(static_cast<int>(1000.0f / (frame_time * 1000.0f)))).c_str());
	glutPostRedisplay(); // 렌더링 수행 요청
}