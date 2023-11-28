#pragma once

#include "common.hpp"

#include <filesystem>

#include "cs_std/graphics/model.hpp"

CS_NAMESPACE_BEGIN
{
	cs_std::graphics::model LoadOBJ(const std::filesystem::path& path);
	cs_std::graphics::model LoadGLTF(const std::filesystem::path& path);
}