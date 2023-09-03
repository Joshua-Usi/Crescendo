#pragma once

#include <vector>
#include <filesystem>

namespace Crescendo::IO
{
	/// <summary>
	/// This class breaks RAII concepts but at least it avoids memcpys
	/// </summary>
	struct Image
	{
		std::vector<uint8_t> pixels;
		uint32_t width, height, channels;
		bool isTransparent;
	};

	Image LoadImage(const std::filesystem::path& path, bool flipOnLoad = true);
}