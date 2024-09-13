#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec4 iTangent;
layout (location = 3) in vec2 iTexCoord;

layout (location = 0) out vec3 oPosition_ws;
layout (location = 1) out vec2 oTexCoord;
layout (location = 2) out mat3 oTBN;

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformData[]; });
void main()
{
	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
	const mat4 m = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];

	mat3 normal = transpose(inverse(mat3(m)));
	vec3 T = normalize(normal * iTangent.xyz);
	vec3 N = normalize(normal * iNormal);
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T) * -iTangent.w;

	oPosition_ws = (m * vec4(iPosition, 1.0f)).xyz;
	oTexCoord = iTexCoord;
	oTBN = mat3(T, B, N);
	
	gl_Position = vp * m * vec4(iPosition, 1.0f);
}
