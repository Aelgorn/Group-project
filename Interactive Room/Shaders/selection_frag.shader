#version 330 core

uniform int id;
out vec4 selectColor;

void main()
{
	selectColor = vec4(id / 255.0f, 0, 0, 1);
}