#include "ImageLoader.hpp"

#include "Core/common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Crescendo::IO
{
	cs_std::image LoadImage(const std::filesystem::path& path)
	{
		constexpr int FIXED_CHANNELS = 4;
		int width, height, channels;
		// Force 4 channel RGBA
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		CS_ASSERT(pixels != nullptr, "Failed to load image: " + path.string());

		// Optional downsampling code
		// Nearest neighbour downsampling
		constexpr uint32_t scaleFactor = 2;
		std::vector<uint8_t> newPixels(width * height * FIXED_CHANNELS / (scaleFactor * scaleFactor));
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
		return cs_std::image(newPixels, width / scaleFactor, height / scaleFactor, FIXED_CHANNELS);
	}
}
