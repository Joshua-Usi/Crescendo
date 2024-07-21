#pragma once

#include "common.hpp" 

#include <filesystem>

#include "cs_std/image.hpp"

CS_NAMESPACE_BEGIN
{
	cs_std::image LoadImage(const std::filesystem::path& path);
}