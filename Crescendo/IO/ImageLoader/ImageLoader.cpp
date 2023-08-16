#include "ImageLoader.hpp"

#include "Core/common.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace Crescendo::IO
{
	Image LoadImage(const std::filesystem::path& path)
	{
		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;

		// Force 4 channel RGBA
		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		
		CS_ASSERT(pixels != nullptr, "Failed to load image: " + path.string());

		// MMM I don't like how we have to copy but it is what it is
		// At least this memory is safe and will be freed
		std::vector<uint8_t> data;
		data.insert(data.end(), pixels, pixels + (width * height * 4));
		Image image(data, width, height, channels);

		stbi_image_free(pixels);

		return image;
	}
}
