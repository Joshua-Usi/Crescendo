#pragma once

#include <filesystem>

#include "cs_std/image.hpp"

namespace Crescendo::IO
{
	cs_std::image LoadImage(const std::filesystem::path& path);
}