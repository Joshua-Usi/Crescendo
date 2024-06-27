#version 460
#include "bindless.glsl"

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

RegisterUniform(Camera, {
	mat4 viewProjection;
});

layout(set = 1, binding = 0) uniform DrawParameters {
	uint camera;
	uint skyboxTexture;
	uint pad0;
	uint pad1;
};

void main()
{
	mat4 vp = GetResource(Camera, camera).viewProjection;
	vec4 position = vp * vec4(iPosition, 1.0f);
	gl_Position = position.xyww;

	oTexCoord = iTexCoord;
}
