#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	// Handles includes and preprocessing too
	// Detect file type from extension
	// Thread-safe
	std::vector<uint8_t> GLSLToSPIRV(const std::filesystem::path& file);
}