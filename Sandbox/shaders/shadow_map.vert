#version 450

layout (location = 0) in vec3 iPosition;

layout(set = 0, binding = 0) uniform uniformBuffer {
	mat4 lightSpaceMatrix;
};

layout(push_constant, std430) uniform pushConstants {
	mat4 model;
};

void main() {
	gl_Position = lightSpaceMatrix * model * vec4(iPosition, 1.0);
}