#pragma once

#include <vector>
#include <filesystem>
#include <memory>

typedef unsigned char stbi_uc;

namespace Crescendo::IO
{
	struct Image
	{
		std::shared_ptr<stbi_uc> pixels;
		uint32_t width, height, channels;
		// Automatically frees itself
		Image() = default;
		inline Image(stbi_uc* pixels, uint32_t width, uint32_t height, uint32_t channels);
	};

	Image LoadImage(const std::filesystem::path& path, bool flipOnLoad = false);
}