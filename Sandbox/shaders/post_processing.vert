#version 460
layout (location = 0) out vec2 oTexCoord;

// 6 Vertex positions and texture uvs for a quad that covers the whole screen
vec2 positions[6] = vec2[](vec2(-1.0f, -1.0f), vec2(1.0f, -1.0f), vec2(-1.0f, 1.0f), vec2(-1.0f, 1.0f), vec2(1.0f, -1.0f), vec2(1.0f, 1.0f));
vec2 texCoords[6] = vec2[](vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(0.0f, 1.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f));

void main()
{
	oTexCoord = texCoords[gl_VertexIndex];
	gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
}
