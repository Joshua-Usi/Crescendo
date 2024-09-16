#pragma once
#include "common.hpp" 
#include "cs_std/image.hpp"
#include <filesystem>

CS_NAMESPACE_BEGIN
{
	cs_std::image LoadImage(const std::filesystem::path& path);
}