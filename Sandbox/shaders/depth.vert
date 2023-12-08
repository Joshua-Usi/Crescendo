#version 460

layout (location = 0) in vec3 iPosition;

layout(set = 0, binding = 0) uniform ViewProjection {
	mat4 viewProjection;
};

layout(std140, set = 1, binding = 0) readonly buffer ShaderStorage {
	mat4 modelBuffer[];
};

void main() {
	const mat4 model = modelBuffer[gl_InstanceIndex];

	gl_Position = viewProjection * model * vec4(iPosition, 1.0);
}