#include "ImageLoaders.hpp"

#include "cs_std/file.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <bit>

CS_NAMESPACE_BEGIN
{
	cs_std::image LoadImage(const std::filesystem::path& path)
	{
		int width, height, channels, tmp;

		std::vector<cs_std::byte> data = cs_std::binary_file(path).open().read();
		stbi_info_from_memory(reinterpret_cast<stbi_uc*>(data.data()), static_cast<int>(data.size()), &width, &height, &channels);
		channels = static_cast<int>(std::bit_ceil(static_cast<uint64_t>(channels)));

 		stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data.data()), static_cast<int>(data.size()), &width, &height, &tmp, channels);

		if (pixels == nullptr)
		{
			cs_std::console::error("Failed to load image: " + path.string(), ". Reason: ", stbi_failure_reason());
			return cs_std::image();
		}

		constexpr uint32_t scaleFactor = 1;
		std::vector<uint8_t> newPixels(width * height * channels / (scaleFactor * scaleFactor));
		if constexpr (scaleFactor == 1)
		{
			memcpy(newPixels.data(), pixels, width * height * channels);
		}
		else
		{
			// Optional downsampling code
			// Nearest neighbour downsampling
			for (uint32_t i = 0; i < height / scaleFactor; i++)
			{
				for (uint32_t j = 0; j < width / scaleFactor; j++)
				{
					for (uint32_t k = 0; k < channels; k++)
					{
						newPixels[(i * width / scaleFactor + j) * channels + k] = pixels[((i * width + j) * scaleFactor) * channels + k];
					}
				}
			}
		}
		stbi_image_free(pixels);
		return cs_std::image(newPixels, width / scaleFactor, height / scaleFactor, channels);
	}
}
