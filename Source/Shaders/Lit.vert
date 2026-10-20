//[윤지훈] 쉐이더 버전 3.3 말고 좀 더 올려도 될 듯함
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 uv;

void main()
{
	//[윤지훈] 빛 계산은 월드 좌표계에서 수행하여야 함
	vec4 world = model * vec4(position, 1.0);
	worldPosition = world.xyz;
	//[윤지훈] 노멀의 경우 스케일링 xyz가 균등하지 않으면 이상하게 틀어짐
	// 그래서 변환 행렬 M의 역행렬의 전치행렬을 곱해줘야 함
	worldNormal = transpose(inverse(mat3(model))) * normal;
	uv = texCoord;
	gl_Position = projection * view * world;
}
