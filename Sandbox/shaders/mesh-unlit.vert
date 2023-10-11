#version 450
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoord;

layout (location = 0) out vec2 oTexCoord;

layout(set = 0, binding = 0) uniform uniformBuffer {
	mat4 viewProjection;
};

layout(push_constant, std430) uniform pushConstants {
	mat4 model;
};

void main()
{
	oTexCoord = iTexCoord;	
	gl_Position = viewProjection * model * vec4(iPosition, 1.0f);
}
