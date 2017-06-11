#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 fragPosition;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main()
{
	float ambientStrength = 0.04f;
	vec3 lampLightColor = vec3(0.7);

	vec3 lampPos_1 = vec3(46.0102f, 7.5f, -13.9979f);
	vec3 lampPos_2 = vec3(46.2899f, 7.5f, -39.8267f);
	vec3 lampPos_3 = vec3(16.7871f, 7.5f, -13.4165f);

	vec3 norm = normalize(Normal);

	vec3 lampLightDir_1 = normalize(lampPos_1 - fragPosition);
	vec3 lampLightDir_2 = normalize(lampPos_2 - fragPosition);
	vec3 lampLightDir_3 = normalize(lampPos_3 - fragPosition);

	float lampDiff_1 = max(dot(norm, lampLightDir_1), 0);
	float lampDiff_2 = max(dot(norm, lampLightDir_2), 0);
	float lampDiff_3 = max(dot(norm, lampLightDir_3), 0);

	vec3 ambiant = ambientStrength * lampLightColor;
	vec3 lampLight_1 = lampDiff_1 * lampLightColor;
	vec3 lampLight_2 = lampDiff_2 * lampLightColor;
	vec3 lampLight_3 = lampDiff_3 * lampLightColor;

	FragColor = vec4(ambiant + lampLight_1 + lampLight_2 + lampLight_3, 1) * texture(texture_diffuse1, TexCoords);
}