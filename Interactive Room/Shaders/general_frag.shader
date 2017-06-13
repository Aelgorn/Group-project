#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 fragPosition;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 livLamp1;
uniform vec3 livLamp2;
uniform vec3 bedLamp;
uniform vec3 kitchLamp;
void main()
{
	float ambientStrength = 0.04f;
	vec3 lampLightColor = vec3(1);

	//normalized normal vector
	vec3 norm = normalize(Normal);
	//light direction
	vec3 livLamp1Dir = normalize(livLamp1 - fragPosition);
	vec3 livLamp2Dir = normalize(livLamp2 - fragPosition);
	vec3 bedLampDir = normalize(bedLamp - fragPosition);
	vec3 kitchLampDir = normalize(kitchLamp - fragPosition);
	//distance between fragment and lamp
	float distanceToLivLamp1 = distance(livLamp1, fragPosition);
	float distanceToLivLamp2 = distance(livLamp2, fragPosition);
	float distanceToBedLamp = distance(bedLamp, fragPosition);
	float distanceToKitchLamp = distance(kitchLamp, fragPosition);
	//light distance attenuation factor
	float attenuationLivLamp1 = 1.0f / (1.0f + 0.002f * pow(distanceToLivLamp1, 2));
	float attenuationLivLamp2 = 1.0f / (1.0f + 0.002f  * pow(distanceToLivLamp2, 2));
	float attenuationBedLamp = 1.0f / (1.0f + 0.002f  * pow(distanceToBedLamp, 2));
	float attenuationKitchLamp = 1.0f / (1.0f + 0.002f * pow(distanceToKitchLamp, 2));
	//light angle attenuation factor
	float livLamp1Diff = max(dot(norm, livLamp1Dir), 0);
	float livLamp2Diff = max(dot(norm, livLamp2Dir), 0);
	float bedLampDiff = max(dot(norm, bedLampDir), 0);
	float kitchLampDiff = max(dot(norm, kitchLampDir), 0);
	//light
	vec3 ambiant = ambientStrength * lampLightColor;
	vec3 livLamp1Light = attenuationLivLamp1 * livLamp1Diff * lampLightColor;
	vec3 livLamp2Light = attenuationLivLamp2 * livLamp2Diff * lampLightColor;
	vec3 bedLampLight = attenuationBedLamp * bedLampDiff * lampLightColor;
	vec3 kitchLampLight = attenuationKitchLamp * kitchLampDiff * lampLightColor;

	FragColor = vec4(ambiant + livLamp1Light + livLamp2Light + bedLampLight + kitchLampLight, 1) * texture(texture_diffuse1, TexCoords);
}