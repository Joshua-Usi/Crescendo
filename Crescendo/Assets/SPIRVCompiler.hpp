#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	// Mainly used for the glsl compiler
	struct PreprocessorDefines
	{
		std::string defines;

		PreprocessorDefines& Define(const std::string& macro);
		PreprocessorDefines& Define(const std::string& macro, const std::string& value);
		const std::string& Get() const;
	};
	// Handles includes and preprocessing too
	// Detect file type from extension
	// Thread-safe
	std::vector<uint8_t> GLSLToSPIRV(const std::filesystem::path& file, const PreprocessorDefines& defines = {});
}