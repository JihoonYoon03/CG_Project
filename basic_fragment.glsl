#version 330 core

in vec3 fPos;
in vec3 fNormal;
in vec3 fColor;

out vec4 FragColor;	// 최종 출력 색상

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
//uniform float shininess;

void main() {
	// 앰비언트 조명
	float ambientLight = 0.3;
	vec3 ambient = ambientLight * lightColor;

	// 디퓨즈 조명
	vec3 nVector = normalize(fNormal);
	vec3 lightDir = normalize(lightPos - fPos);
	float diffuseLight = max(dot(nVector, lightDir), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	// 스페큘러 조명
	vec3 viewDir = normalize(viewPos - fPos);
	vec3 reflectDir = reflect(-lightDir, nVector);
	float specularLight = max(dot(viewDir, reflectDir), 0.0);
	specularLight = pow(specularLight, 32.0f);
	vec3 specular = specularLight * lightColor;

	vec3 result = (ambient + diffuse + specular) * fColor;

    FragColor = vec4(result, 1.0);
}