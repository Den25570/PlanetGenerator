#version 330 core

in vec3 color;
out vec4 res_color;

void main()
{
	res_color = vec4(color,1.0f);
}