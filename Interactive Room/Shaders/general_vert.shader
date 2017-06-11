#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 fragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	mat4 MVP = projection * view * model;

	TexCoords = aTexCoords;
	gl_Position = MVP * vec4(aPos, 1.0);
	fragPosition = (model * vec4(aPos, 0)).xyz;
	Normal = mat3(transpose(inverse(model))) * aNormal;
}