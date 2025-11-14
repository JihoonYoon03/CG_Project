#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 colorIn;
out vec3 color;
uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);
uniform mat4 model = mat4(1.0);

void main()
{
	mat4 model_view = view * model;
	gl_Position = projection * model_view * vec4(position, 1.0);
	color = colorIn;
} 