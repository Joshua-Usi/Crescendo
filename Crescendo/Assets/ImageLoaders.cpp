#include "ImageLoaders.hpp"
#include "cs_std/file.hpp"
#include "stb/stb_image.h"
#include <bit>

CS_NAMESPACE_BEGIN
{
	cs_std::image LoadImage(const std::string& path)
	{
		int width, height, channels, tmp;

		cs_std::binary_file file(path);

		if (!file.exists())
		{
			cs_std::console::error("Failed to load image: ", path, ". File does not exist");
			return cs_std::image();
		}

		std::vector<cs_std::byte> data = file.open().read();

		stbi_info_from_memory(reinterpret_cast<stbi_uc*>(data.data()), static_cast<int>(data.size()), &width, &height, &channels);
		channels = static_cast<int>(std::bit_ceil(static_cast<uint64_t>(channels)));
 		stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data.data()), static_cast<int>(data.size()), &width, &height, &tmp, channels);

		if (pixels == nullptr)
		{
			cs_std::console::error("Failed to load image: ", path, ". ", stbi_failure_reason());
			return cs_std::image();
		}

		std::vector<uint8_t> newPixels(width * height * channels);
		memcpy(newPixels.data(), pixels, width * height * channels);
		stbi_image_free(pixels);
		return cs_std::image(newPixels, width, height, channels);
	}
}
