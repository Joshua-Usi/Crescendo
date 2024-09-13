#version 460
#include "bindless.glsl"

struct Glyph
{
	uint textureID;
	float width, height, bearingX, bearingY, advance;
	uint dummy1, dummy2;
};

layout (location = 0) out vec2 oTexCoord;
layout (location = 1) out flat uint oTextureID;

layout (push_constant) uniform PushConstants {
	uint cameraIdx;
	uint transformBufferIdx;
	uint glyphBufferIdx;
	uint characterBufferIdx;
	// cumulativeAdvanceBuffer
	uint advanceBufferIdx;
	uint startingIdx;
	// Offset for alignment
	float horizontalOffset;
	// Offset for line gaps
	float verticalOffset;
	float fontSize;
};

RegisterBuffer(std430, readonly, TransformBuffer, { mat4 transforms[]; });
RegisterBuffer(std430, readonly, CumulativeAdvanceBuffer, { float cumulativeAdvance[]; });
RegisterBuffer(std430, readonly, CharacterBuffer, { uint characters[]; });
RegisterBuffer(std430, readonly, GlyphBuffer, { Glyph glyphs[]; });

void main()
{
	uint vertexIndex = gl_VertexIndex % 6;
	uint characterID = startingIdx + gl_VertexIndex / 6;

	uint textBufferUintIdx = characterID / 4;
	uint textBufferUintOffset = characterID % 4;

	uint packed = GetResource(CharacterBuffer, characterBufferIdx).characters[textBufferUintIdx];
	// Unpack character from uint
	uint character = ((packed >> (textBufferUintOffset * 8)) & 0xFF) - 32;

	const Glyph glyph = GetResource(GlyphBuffer, glyphBufferIdx).glyphs[character];
	oTextureID = glyph.textureID;

	const mat4 vp = GetResource(TransformBuffer, transformBufferIdx).transforms[cameraIdx];
	const mat4 model = GetResource(TransformBuffer, transformBufferIdx).transforms[gl_InstanceIndex];

	oTexCoord = vec2(vertexIndex & 1, mod((vertexIndex + 1) / 3, 2));

	vec2 vertexPosition = oTexCoord * vec2(glyph.width, glyph.height) + vec2(glyph.bearingX + GetResource(CumulativeAdvanceBuffer, advanceBufferIdx).cumulativeAdvance[characterID] + horizontalOffset, glyph.bearingY + verticalOffset);
	gl_Position = vp * model * vec4(vertexPosition * fontSize, 0.0, 1.0);
}
