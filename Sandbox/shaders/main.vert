#version 460
#include "bindless.glsl"
#include "permutations.glsl"

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
#ifdef USE_NORMAL_MAP
	layout (location = 2) in vec4 iTangent;
#endif
#ifdef USE_TEXTURE_MAPS
	layout (location = 3) in vec2 iTexCoord;
#endif 

layout (location = 0) out vec3 oPosition_ws;
#ifdef USE_TEXTURE_MAPS
	layout (location = 1) out vec2 oTexCoord;
#endif
#ifdef USE_NORMAL_MAP
	layout (location = 2) out mat3 oTBN;
#else
	layout (location = 2) out vec3 oNormal;
#endif

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformData[]; });

void main()
{
	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
	const mat4 m = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];
	#ifdef USE_NORMAL_MAP
		mat3 normal = transpose(inverse(mat3(m)));
		vec3 T = normalize(normal * iTangent.xyz);
		vec3 N = normalize(normal * iNormal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T) * -iTangent.w;
		oTBN = mat3(T, B, N);
	#else
		oNormal = normalize(transpose(inverse(mat3(m))) * iNormal);
	#endif 
	#ifdef USE_TEXTURE_MAPS
		oTexCoord = iTexCoord;
	#endif
	oPosition_ws = (m * vec4(iPosition, 1.0f)).xyz;
	gl_Position = vp * m * vec4(iPosition, 1.0f);
}
