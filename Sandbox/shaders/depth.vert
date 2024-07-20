#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformBuffer[]; });

void main() {
	const mat4 model = GetResource(TransformBuffer, transformBufferIdx).transformBuffer[gl_InstanceIndex];
	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformBuffer[cameraIdx];
	gl_Position = vp * model * vec4(iPosition, 1.0);
}