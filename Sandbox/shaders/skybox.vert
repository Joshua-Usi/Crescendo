#version 460
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

// Should not have any translation
layout(set = 0, binding = 0) uniform ViewProjection {
	mat4 viewProjection;
};

void main()
{
	vec4 position = viewProjection * vec4(iPosition, 1.0f);
	gl_Position = position.xyww;

	oTexCoord = iTexCoord;
}
