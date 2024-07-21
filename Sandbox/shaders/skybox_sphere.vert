#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformBuffer[]; });
void main()
{
	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformBuffer[cameraIdx];
	vec4 position = vp * vec4(iPosition, 1.0f);
	gl_Position = position.xyww;

	oTexCoord = iTexCoord;
}
