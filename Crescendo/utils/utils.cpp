#include "utils.hpp"
#include "cs_std/file.hpp"
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
	std::string GLSLPreprocessorIncludesRecursive(const std::filesystem::path& filePath, std::set<std::filesystem::path>& included_files)
	{
		// Prevent infinite recursion due to circular includes
		if (included_files.count(filePath) > 0)
			return "";
		included_files.insert(filePath);
		cs_std::text_file tf(filePath);
		if (!tf.exists())
			throw std::runtime_error("File not found: " + filePath.string());
		tf.open();

		std::string content = tf.read();
		std::istringstream iss(content);
		std::string line;
		std::string result;

		while (std::getline(iss, line)) {
			// Trim leading whitespaces
			line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
				return !std::isspace(ch);
				}));

			// If the line doesn't start with '#include', append it as is
			if (line.compare(0, 8, "#include") != 0) {
				result += line + "\n";
				continue;
			}

			// Process the '#include' directive
			size_t start_pos = line.find_first_of("\"<", 8);
			if (start_pos == std::string::npos) {
				throw std::runtime_error("Invalid #include syntax in file: " + filePath.string());
			}

			char delimiter = line[start_pos];
			size_t end_pos = line.find_first_of(delimiter == '"' ? "\"" : ">", start_pos + 1);
			if (end_pos == std::string::npos) {
				throw std::runtime_error("Invalid #include syntax in file: " + filePath.string());
			}

			std::string filename = line.substr(start_pos + 1, end_pos - start_pos - 1);
			std::filesystem::path include_path;

			if (delimiter == '"') {
				include_path = filePath.parent_path() / filename;
			}
			else {
				throw std::runtime_error("System includes are not supported: " + filename);
			}

			// Recursively process the included file
			std::string included_content = GLSLPreprocessorIncludesRecursive(include_path, included_files);

			// Replace the '#include' line with the included content
			result += included_content;
		}

		return result;
	}
	std::string GLSLPreprocessorIncludes(const std::filesystem::path& filePath)
	{
		// prevent multiple includes
		std::set<std::filesystem::path> includedFiles;
		return GLSLPreprocessorIncludesRecursive(filePath, includedFiles);
	}
}