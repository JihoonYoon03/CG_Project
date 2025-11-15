#version 330 core

// Vertex Shader
// vPos와 vColor는 각각 셰이더 속성 변수이다.
layout(location = 0) in vec3 vPos;     // VBO_position에서 받은 위치 (location이 0으로 설정된 셰이더 속성)
layout(location = 1) in vec3 vColor;   // VBO_color에서 받은 색상 (location이 1로 설정된 셰이더 속성)

out vec3 fColor;  // 프래그먼트 셰이더로 전달할 값 (out으로 보낸 변수를 in으로 받을 때, 두 변수의 이름은 서로 같아야 한다.)

uniform mat4 model_Transform; // 복합 변환 행렬 선언
uniform mat4 view_Transform; // 뷰 변환
uniform mat4 projection_Transform; // 투영 변환

void main() {
    gl_Position = projection_Transform * view_Transform * model_Transform * vec4(vPos, 1.0);
    fColor = vColor;
}