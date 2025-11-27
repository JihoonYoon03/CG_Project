#version 330 core

// Vertex Shader
// vPos�� vColor�� ���� ���̴� �Ӽ� �����̴�.
layout(location = 0) in vec3 vPos;     // VBO_position���� ���� ��ġ (location�� 0���� ������ ���̴� �Ӽ�)
layout(location = 1) in vec3 vColor;   // VBO_color���� ���� ���� (location�� 1�� ������ ���̴� �Ӽ�)

out vec3 fColor;  // �����׸�Ʈ ���̴��� ������ �� (out���� ���� ������ in���� ���� ��, �� ������ �̸��� ���� ���ƾ� �Ѵ�.)

uniform mat4 projection; // ���� ��ȯ
uniform mat4 view; // �� ��ȯ
uniform mat4 model; // ���� ��ȯ ��� ����

void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
    fColor = vColor;
}