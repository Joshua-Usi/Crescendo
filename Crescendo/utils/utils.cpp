#include "utils.hpp"
#include "cs_std/file.hpp"
#include <filesystem>
#include <set>

CS_NAMESPACE_BEGIN
{
	std::vector<std::string> GetFonts(const std::string& fontDir)
	{
		if (!std::filesystem::exists(fontDir))
		{
			cs_std::console::warn("Could not find font directory: ", fontDir, ". No fonts were found");
			return {};
		}

		if (!std::filesystem::is_directory(fontDir))
		{
			cs_std::console::warn("Provided font directory is not a directory: ", fontDir, ". No fonts were found");
			return {};
		}
		std::vector<std::string> fontPaths;
		for (const auto& entry : std::filesystem::directory_iterator(fontDir))
		{
			if (!entry.is_regular_file())
				continue;
			// We only care about true type fonts
			if (entry.path().extension() != ".ttf")
				continue;
			fontPaths.push_back(entry.path().string());
		}
		return fontPaths;
	}
}