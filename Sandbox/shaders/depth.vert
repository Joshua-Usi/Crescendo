#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformData[]; });

void main() {
	const mat4 m = GetResource(TransformBuffer, transformBufferIdx).transformData[gl_InstanceIndex];
	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformData[cameraIdx];
	gl_Position = vp * m * vec4(iPosition, 1.0);
}