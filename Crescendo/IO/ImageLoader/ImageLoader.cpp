#include "ImageLoader.hpp"

#include "Core/common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Crescendo::IO
{
	Image LoadImage(const std::filesystem::path& path)
	{
		constexpr int FIXED_CHANNELS = 4;
		int width, height, channels;
		// Force 4 channel RGBA
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		CS_ASSERT(pixels != nullptr, "Failed to load image: " + path.string());

		// Optional downsampling code
		// Nearest neighbour downsampling
		/*uint32_t scaleFactor = 2;
		stbi_uc* newPixels = new stbi_uc[width * height * FIXED_CHANNELS / (scaleFactor * scaleFactor)];
		for (uint32_t i = 0; i < height / scaleFactor; i++)
		{
			for (uint32_t j = 0; j < width / scaleFactor; j++)
			{
				for (uint32_t k = 0; k < FIXED_CHANNELS; k++)
				{
					newPixels[(i * width / scaleFactor + j) * FIXED_CHANNELS + k] = pixels[((i * width + j) * scaleFactor) * FIXED_CHANNELS + k];
				}
			}
		}
		stbi_image_free(pixels);
		return Image(newPixels, width / scaleFactor, height / scaleFactor, FIXED_CHANNELS);*/

		return Image(pixels, width, height, FIXED_CHANNELS);
	}
	inline Image::Image(stbi_uc* pixels, uint32_t width, uint32_t height, uint32_t channels)
		: pixels(pixels, [](stbi_uc* p) { if (p) stbi_image_free(p); /*delete p;*/ }), width(width), height(height), channels(channels) {}
}
