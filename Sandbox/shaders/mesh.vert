#version 450
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec4 iTangent;
layout (location = 3) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;
layout (location = 1) out vec3 oNormal;
layout (location = 2) out vec3 oTanFragPos;
layout (location = 3) out vec3 oTanLightPos;
layout (location = 4) out vec3 oTanViewPos;
layout (location = 5) out vec4 oFragPosLightSpace;

layout(set = 0, binding = 0) uniform uniformBuffer {
	mat4 viewProjection;
	mat4 lightSpaceMatrix;
};

layout(set = 0, binding = 1) uniform blinnPhongLightingData {
	vec4 lightPosition;
	vec4 viewPosition;
};

layout(push_constant, std430) uniform pushConstants {
	mat4 model;
};

const mat4 bias = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0
);

void main()
{
	oTexCoord = iTexCoord;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * iTangent.xyz);
	vec3 N = normalize(normalMatrix * iNormal);
	oNormal = N;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
	oTanLightPos = TBN * lightPosition.xyz;
	oTanViewPos = TBN * viewPosition.xyz;
	oTanFragPos = TBN * vec3(model * vec4(iPosition, 1.0));

	oFragPosLightSpace = (bias * lightSpaceMatrix * model) * vec4(iPosition, 1.0f);
	
	gl_Position = viewProjection * model * vec4(iPosition, 1.0f);
}
