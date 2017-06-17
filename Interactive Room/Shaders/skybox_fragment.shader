#version 330 core

in vec3 skyboxCoord;

out vec4 color;

uniform samplerCube skyboxTexture;

void main()
{
	//color = texture(skyboxTexture, skyboxCoord);
	color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
} 