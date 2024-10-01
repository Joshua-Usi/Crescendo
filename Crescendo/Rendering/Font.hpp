#pragma once
#include "common.hpp"
#include "Rendering/RenderResourceManager.hpp"

struct stbtt_fontinfo;

CS_NAMESPACE_BEGIN
{
	class Font
	{
	public:
		struct Character
		{
			struct ShaderRepresentation
			{
				uint32_t texture;
				float width, height;
				float bearingX, bearingY;
				float advance;
				uint32_t dummy1, dummy2;
			};
			Vulkan::TextureHandle texture;
			float width, height;
			float bearingX, bearingY;
			float advance;
			uint32_t dummy1 = 0, dummy2 = 0; // pad to 32 bytes, can be used later

			ShaderRepresentation GetShaderRepresentation() const { return { texture.GetIndex(), width, height, bearingX, bearingY, advance, 0, 0 }; }
		};
	private:
		RenderResourceManager* resourceManager;
	public:
		float ascent, descent, lineGap, lineHeight;
		std::vector<Character> characters;
		Vulkan::SSBOBufferHandle characterDataBufferHandle;
	private:
		std::vector<Character::ShaderRepresentation> GetShaderRepresentation() const;
		cs_std::image GenerateMSDF(stbtt_fontinfo& fontInfo, int32_t glyphIndex, uint32_t borderWidth, float scale);
	public:
		Font() = default;
		Font(const std::vector<uint8_t>& ttfData, RenderResourceManager& resourceManager);
		~Font();
		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;
		Font(Font&& other) noexcept;
		Font& operator=(Font&& other) noexcept;
		std::vector<float> GenerateCumulativeAdvance(const std::string& text, uint32_t start = 0, uint32_t count = UINT32_MAX) const;
	};
}