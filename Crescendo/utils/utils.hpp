#pragma once
#include "common.hpp"

#include <filesystem>

CS_NAMESPACE_BEGIN
{
	struct glslcOptions
	{
		std::string options;

		glslcOptions& AddOption(const std::string& option)
		{
			options += " " + option;
			return *this;
		}
		operator const std::string&() const { return options; }
	};

	std::vector<std::string> GetFonts(const std::string & fontDir);
	// Does not support additional include directories
	// Reads additional files from the same directory as the source file
	std::string GLSLPreprocessorIncludes(const std::filesystem::path& sourceFile);
}