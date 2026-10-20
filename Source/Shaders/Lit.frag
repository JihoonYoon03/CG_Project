#version 330 core
const int MAX_LIGHTS = 8;

struct Light
{
	int type;
	vec3 color;
	//[윤지훈] 빛 강도
	float intensity;
	vec3 position;
	vec3 direction;
	//[윤지훈] 빛 범위
	float range;
	//[윤지훈] 스포트라이트에서 가장 밝은 안쪽 부분 각도와 최대 빛 각도를
	//framework.cpp에서 std::cos(glm::radians(inner)) 로 계산해서 넘겨줌
	//즉 0도일 때 1.0, 90도일 때 0.0인 값임. 라이트 구조체의 각도와는 다름에 유의
	float innerCos;
	float outerCos;
};

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;
//[윤지훈] 환경광 기본 색상
uniform vec3 ambientLight;
//[윤지훈] 현재 보고있는 위치(반사광 계산에 사용)
uniform vec3 viewPosition;
//[윤지훈] 재질 기본 색상
uniform vec3 materialColor;
//[윤지훈] 반짝임 계수(반사광에 적용)
uniform float shininess;
//[윤지훈] true 시 빛 적용 없이 단색 출력
uniform bool unlit;
uniform bool ambientOnly;
uniform bool useTexture;
//[윤지훈] 텍스처 샘플러. cpu 코드에서 지정된 텍스처 유닛과 uv값 받아서 해당 위치 색상 리턴해줌
// glGetUniformLocation으로 colorTexture 위치를 따로 저장하고
// glUniform1i(colorTexture 위치 변수, 0); 이런식으로 지정해주면 GL_TEXTURE0에 대한 샘플러로 설정됨
// 셰이더 버전 4.2 이상부턴 layout(binding = 0) 이런 식으로 셰이더 코드에서 바로 지정 가능함.
uniform sampler2D colorTexture;
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;
out vec4 fragColor;

//[윤지훈] 벡터 길이 0이면 정규화 안 됨(벡터 / 벡터 길이 이므로).
//기본 normalize는 영벡터에 대한 처리 안해줌
vec3 SafeNormalize(vec3 value)
{
	return dot(value, value) > 0.0 ? normalize(value) : vec3(0.0);
}

void main()
{
	vec4 base = vec4(materialColor, 1.0);

	if (useTexture)
	{
		base = texture(colorTexture, uv);
	}

	//[윤지훈] 광원 미적용 시 색상을 바로 리턴
	if (unlit)
	{
		fragColor = base;
		return;
	}

	//[윤지훈] 환경광만 적용하고 리턴
	if (ambientOnly)
	{
		fragColor = vec4(ambientLight * base.rgb, base.a);
		return;
	}

	//[윤지훈] 단위 노멀벡터 구하기
	vec3 n = SafeNormalize(worldNormal);
	//[윤지훈] 지금 계산하고있는 점에서 카메라 위치를 향하는 방향벡터
	vec3 viewDirection = SafeNormalize(viewPosition - worldPosition);
	//[윤지훈] result는 조명 계산 최종값임. 환경광은 아무 계산 없이 즉시 적용임
	vec3 result = ambientLight;

	//[윤지훈] 빛 개수만큼 연산 반복
	for (int i = 0; i < lightCount; ++i)
	{
		//[윤지훈] 현재 지점에서 빛을 향하는 방향벡터
		vec3 toLight;
		//[윤지훈] 거리에 따른 빛 감쇠 정도
		float attenuation = 1.0;

		//[윤지훈] 방향광(태양)인 경우 즉시 계산
		if (lights[i].type == 0)
		{
			toLight = -SafeNormalize(lights[i].direction);
		}
		else
		{
			//[윤지훈] 현재 위치에서 빛까지의 벡터
			vec3 offset = lights[i].position - worldPosition;
			//[윤지훈] 빛까지의 거리(스칼라)로 변환. distance 변수명 바꿔야할듯
			float distance = length(offset);
			toLight = SafeNormalize(offset);

			//[윤지훈] 빛 범위가 0이 아닌 경우에만 계산
			if (lights[i].range > 0.0)
			{
				//[윤지훈] 감쇠하는 정도는 0.0 ~ 1.0으로 고정됨
				float falloff = max(1.0 - distance / lights[i].range, 0.0);
				//[윤지훈] 거리가 멀어질 수록 점점 급격히 어두워지도록 한 번 제곱함
				attenuation = falloff * falloff;
			}

			//[윤지훈] 스포트라이트인 경우
			if (lights[i].type == 2)
			{
				//[윤지훈] 현재 스포트라이트 방향과 스포트라이트->계산하고있는 위치까지의 방향이 얼마나 일치하는지 내적 연산
				// 완전 일치하면 1, 90도부터는 0 이하로 내려감
				float angleCos = dot(-toLight, SafeNormalize(lights[i].direction));
				//[윤지훈] smoothStep은 Hermite 보간이라는 그래프를 써서
				// 입력값을 지정 범위를 기준으로 0.0 ~ 1.0 사이로 보간해줌
				// 인자는 순서대로 최소값, 최대값, 입력값임
				// ex) 최대 각도 45도, 안쪽 각도 0도이고 angleCos가 0.55이면
				// outerCos = cos(45) = 0.5, innerCos = cos(0) = 1.0 로, 0.5~1.0 범위가 지정됨
				attenuation *= smoothstep(lights[i].outerCos, lights[i].innerCos, angleCos);
			}
		}

		//[윤지훈] 디퓨즈 항, 스페큘러 항 계산
		float diffuse = max(dot(n, toLight), 0.0);
		float specular = pow(max(dot(viewDirection, reflect(-toLight, n)), 0.0), max(shininess, 1.0));
		//[윤지훈] 감쇠 고려하여 빛 색상 계산
		vec3 lightColor = lights[i].color * lights[i].intensity * attenuation;
		//[윤지훈] 빛 색상에 디퓨즈, 스페큘러 계산한 결과 더함
		result += lightColor * diffuse;
		result += lightColor * specular;
	}

	//[윤지훈] 텍스쳐 색상에 빛 계산 후의 색상을 적용하여 리턴
	fragColor = vec4(base.rgb * result, base.a);
}
