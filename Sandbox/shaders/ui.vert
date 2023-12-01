#version 460

layout (location = 0) out vec2 oTexCoord;

layout(set = 0, binding = 0) uniform uniformBuffer {
	mat4 viewProjection;
};

layout(std140, set = 1, binding = 0) readonly buffer ShaderStorage {
	mat4 modelBuffer[];
};

void main()
{
	const mat4 model = modelBuffer[gl_InstanceIndex];
	oTexCoord = vec2(gl_VertexIndex & 1, mod((gl_VertexIndex + 1) / 3, 2));
	gl_Position = viewProjection * model * vec4(oTexCoord - 0.5f, 0.0f, 1.0f);
}
