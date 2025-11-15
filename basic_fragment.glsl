#version 330 core

// Fragment Shader
in vec3 fColor;       // Vertex Shader로부터 보간된 값이 들어옴 (out으로 전달한 fColor와 변수명을 일치시켜줘야 하기 때문에 fColor로 속성 변수명 설정하고 in으로 받아옴)
out vec4 FragColor;   // 최종 픽셀 색상 (out을 통해 프레임 버퍼(화면)에 저장되도록 함)

void main() {
    FragColor = vec4(fColor, 1.0); // 픽셀 색상 = 보간된 정점 색상 (이 또한 행렬 변환에 용이하도록 하기 위해 동차 좌표로 설정). 이후 out으로 인해 모니터에 해당 색상이 출력된다.
}