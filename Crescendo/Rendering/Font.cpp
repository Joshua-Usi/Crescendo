#include "Font.hpp"
#include "stb_truetype.h"
#include "msdf_c/msdf.h"

CS_NAMESPACE_BEGIN
{
	Font::Font(const std::vector<uint8_t>& ttfData, Vulkan::ResourceManager& resourceManager)
		// 95 printable ASCII characters
		: characters(95)
	{
		stbtt_fontinfo fontInfo;
		if (!stbtt_InitFont(&fontInfo, ttfData.data(), 0)) cs_std::console::error("Failed to initialise font");

		uint32_t borderWidth = 4;

		uint32_t fontScale = 40;
		float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontScale);
		float scaleDivisor = 1.0f / fontScale;

		int32_t ascent, descent, lineGap;
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
		this->ascent = ascent * scale * scaleDivisor;
		this->descent = descent * scale * scaleDivisor;
		this->lineGap = lineGap * scale * scaleDivisor;
		this->lineHeight = (this->ascent - this->descent + this->lineGap);

		// Loop over each ascii character
		for (int32_t codepoint = 32; codepoint < 126; codepoint++)
		{
			int32_t glyphIndex = stbtt_FindGlyphIndex(&fontInfo, codepoint);
			if (glyphIndex == 0) continue;

			Font::Character character;

			int32_t advanceWidth, leftSideBearing;
			stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advanceWidth, &leftSideBearing);
			character.advance = advanceWidth * scale * scaleDivisor;
			character.bearingX = leftSideBearing * scale * scaleDivisor;

			int32_t c_x1, c_y1, c_x2, c_y2;
			stbtt_GetGlyphBitmapBox(&fontInfo, glyphIndex, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
			c_x1 -= borderWidth; c_y1 -= borderWidth; c_x2 += borderWidth; c_y2 += borderWidth;

			uint32_t width = c_x2 - c_x1;
			uint32_t height = c_y2 - c_y1;

			character.width = width * scaleDivisor;
			character.height = height * scaleDivisor;
			character.bearingY = c_y1 * scaleDivisor;
			if (width - 2 * borderWidth != 0 && height - 2 * borderWidth != 0)
			{
				cs_std::image glyph = GenerateMSDF(fontInfo, glyphIndex, borderWidth, scale);
				character.texture = resourceManager.UploadTexture(glyph, {
					Vulkan::ResourceManager::Colorspace::Linear, 1.0f, false
				});
			}
			characters[codepoint - 32] = character;
		}

		std::vector<Font::Character::ShaderRepresentation> characterData = GetShaderRepresentation();
		characterDataBufferHandle = resourceManager.CreateBuffer(sizeof(Font::Character::ShaderRepresentation) * characterData.size(), VK_SHADER_STAGE_ALL);
		resourceManager.GetBuffer(characterDataBufferHandle).buffer.memcpy(characterData.data(), sizeof(Font::Character::ShaderRepresentation) * characterData.size());
	}
	cs_std::image Font::GenerateMSDF(stbtt_fontinfo& fontInfo, int32_t glyphIndex, uint32_t borderWidth, float scale)
	{
		msdf_Result msdfResult;
		uint32_t result = msdf_genGlyph(&msdfResult, &fontInfo, glyphIndex, borderWidth, scale, 40.0f, nullptr);
		if (result == 0)
		{
			cs_std::console::error("Failed to generate msdf for glyph");
			return cs_std::image();
		}

		cs_std::image glyph(msdfResult.width, msdfResult.height, 4);
		for (uint32_t i = 0, j = 0; i < msdfResult.width * msdfResult.height * 3; i += 3, j += 4)
		{
			glyph.data[j] = (*(msdfResult.rgb + i)) * 255;
			glyph.data[j + 1] = (*(msdfResult.rgb + i + 1)) * 255;
			glyph.data[j + 2] = (*(msdfResult.rgb + i + 2)) * 255;
			glyph.data[j + 3] = 255;
		}
		return glyph;
	}
	std::vector<Font::Character::ShaderRepresentation> Font::GetShaderRepresentation() const
	{
		std::vector<Character::ShaderRepresentation> data;
		data.reserve(characters.size());
		for (const Character& c : characters)
			data.push_back(c.GetShaderRepresentation());
		return data;
	}
	std::vector<float> Font::GenerateCumulativeAdvance(const std::string& text, uint32_t start, uint32_t count) const
	{
		count = (count > text.size()) ? text.size() : count;
		std::vector<float> data;
		data.reserve(count);
		data.push_back(0.0f);
		// Reserve early, if there are spaces, it's likely we won't use all of the space
		float cumulativeAdvance = 0.0f;
		for (size_t i = start; i < count - 1; i++)
		{
			const char c = text[i];
			if (c < 32 || c > 126) continue;
			if (c == 32)
			{
				cumulativeAdvance += characters[0].advance;
				data.back() = cumulativeAdvance;
				continue;
			}
			cumulativeAdvance += characters[c - 32].advance;
			data.push_back(cumulativeAdvance);
		}
		return data;
	}
}