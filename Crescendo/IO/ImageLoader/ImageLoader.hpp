#pragma once

#include <vector>
#include <filesystem>

#include <iostream>

namespace Crescendo::IO
{
	struct Image
	{
		void* pixels;
		uint32_t width, height, channels;
		// Automatically frees itself
		~Image();
	};

	Image LoadImage(const std::filesystem::path& path, bool flipOnLoad = false);
}