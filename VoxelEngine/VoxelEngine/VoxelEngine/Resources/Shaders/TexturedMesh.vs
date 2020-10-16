#version 460 core

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

uniform mat4 MVP;
uniform mat4 Model;
uniform mat4 InvTrModel;

out vec3 fragPos;
out vec4 Normal;
out vec2 TexCoord;

void main()
{
	TexCoord = vertTexCoord;

	vec3 norm = normalize(vertNormal);
	fragPos = vec3(Model * vec4(vertPosition, 1.0f));
	Normal = InvTrModel * vec4(norm, 1.0f); 
	Normal = normalize(Normal);

	gl_Position = MVP * vec4(vertPosition, 1.0f);
}