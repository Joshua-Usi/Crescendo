#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;


RegisterUniform(Camera, {
	mat4 viewProjection;
});

RegisterBuffer(std430, readonly, ModelBuffer, {
	mat4 modelBuffer[];
});



void main() {
	const mat4 model = GetResource(ModelBuffer, 0).modelBuffer[gl_InstanceIndex];
	const mat4 vp = GetResource(Camera, 0).viewProjection;
	gl_Position = vp * model * vec4(iPosition, 1.0);
}