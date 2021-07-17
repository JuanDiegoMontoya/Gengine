#version 460 core

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec2 vertTexCoord;

layout(location = 0) uniform mat4 MVP;
layout(location = 1) uniform mat4 Model;
layout(location = 2) uniform mat4 InvTrModel;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 Normal;
layout(location = 2) out vec2 TexCoord;

void main()
{
	TexCoord = vertTexCoord;

	vec3 norm = normalize(vertNormal);
	fragPos = vec3(Model * vec4(vertPosition, 1.0f));
	//Normal = (InvTrModel * vec4(norm, 1.0)).xyz; 
	//Normal = normalize(Normal);
	Normal = norm;

	gl_Position = MVP * vec4(vertPosition, 1.0f);
}