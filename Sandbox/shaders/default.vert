#version 460
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec4 iTangent;
layout (location = 3) in vec2 iTexCoord;

layout (location = 0) out vec3 oPosition_ws;
layout (location = 1) out vec2 oTexCoord;
layout (location = 2) out mat3 oTBN;

layout(set = 0, binding = 0) uniform ViewProjection { mat4 viewProjection; };

layout(std140, set = 1, binding = 0) readonly buffer ShaderStorage { mat4 modelBuffer[]; };

void main()
{
	const mat4 modelMatrix = modelBuffer[gl_InstanceIndex];

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec3 T = normalize(normalMatrix * iTangent.xyz);
	vec3 N = normalize(normalMatrix * iNormal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	oPosition_ws = (modelMatrix * vec4(iPosition, 1.0f)).xyz;
	oTexCoord = iTexCoord;
	oTBN = transpose(mat3(T, B, N));
	
	gl_Position = viewProjection * modelMatrix * vec4(iPosition, 1.0f);
}
