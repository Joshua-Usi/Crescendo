#pragma once

#include <vector>
#include <filesystem>
#include <memory>

typedef unsigned char stbi_uc;

namespace Crescendo::IO
{
	struct Image
	{
		std::unique_ptr<stbi_uc[], void(*)(stbi_uc*)> pixels;
		uint32_t width, height, channels;
		// Automatically frees itself
		inline Image(stbi_uc* pixels = nullptr, uint32_t width = 0, uint32_t height = 0, uint32_t channels = 0);
	};

	Image LoadImage(const std::filesystem::path& path);
}