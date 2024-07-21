#version 460
#include "bindless.glsl"

layout (location = 0) out vec2 oTexCoord;

void main()
{
	oTexCoord = vec2(gl_VertexIndex & 1, mod((gl_VertexIndex + 1) / 3, 2));
	gl_Position = vec4((oTexCoord - 0.5f) * 2.0f, 0.0f, 1.0f);
}
