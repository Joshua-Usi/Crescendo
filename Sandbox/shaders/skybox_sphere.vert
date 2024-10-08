#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

layout (push_constant) uniform PushConstants {
	uint cameraIdx; // Seperate to the active camera's view projection as this one requires no translation
	uint transformBufferIdx;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transformBuffer[]; });

void main() {
	oTexCoord = iTexCoord;

	mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transformBuffer[cameraIdx];
	// Remove translation	
	vp[3][0] = 0.0; vp[3][1] = 0.0; vp[3][3] = 0.0;
	gl_Position = (vp * vec4(iPosition, 1.0f)).xyww;
}
