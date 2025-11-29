#version 330 core

layout(location = 0) in vec3 vPos;     // 정점 위치
layout(location = 1) in vec3 vColor;   // 정점 색상
layout(location = 2) in vec3 vNormal;  // 노멀 벡터

out vec3 fPos;	// Fragment Shader로 보낼 위치 데이터
out vec3 fNormal;	// Fragment Shader로 보낼 노멀 데이터
out vec3 fColor;    // Fragment Shader로 보낼 색상 데이터

uniform mat4 projection; // 투영 변환
uniform mat4 view;      // 카메라 변환
uniform mat4 model;     // 월드(모델) 변환

void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
	fPos = vec3(model * vec4(vPos, 1.0));
    fNormal = transpose(inverse(mat3(model))) * vNormal;
    fColor = vColor;
}