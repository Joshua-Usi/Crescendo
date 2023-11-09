#version 460
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

layout(set = 0, binding = 0) uniform ViewProjection {
	mat4 viewProjection;
};

layout (push_constant) uniform PushConstant {
	mat4 model;
};

void main()
{
	vec4 position = viewProjection * model * vec4(iPosition, 1.0f);
	gl_Position = position;

	oTexCoord = iTexCoord;
}
