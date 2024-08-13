#include "Cvar.hpp"

#include "cs_std/xml/xml.hpp"
#include "cs_std/file.hpp"

#include <regex>

CS_NAMESPACE_BEGIN
{
	enum class CVarTypeFlag
	{
		INT,
		FLOAT,
		STRING,
		BOOL
	};
	CVarTypeFlag DetectType(const std::string& value)
	{
		std::string temp = value;
		std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
		if (temp == "true" || temp == "false") return CVarTypeFlag::BOOL;
		if (std::regex_match(value, std::regex("^-?[0-9]+$"))) return CVarTypeFlag::INT;
		if (std::regex_match(value, std::regex("^[-+]?[0-9]*\\.?[0-9]+$"))) return CVarTypeFlag::FLOAT;
		return CVarTypeFlag::STRING;
	}
	std::unordered_map<std::string, CVarType> CVar::data {};
	std::vector<std::string> CVar::GetNames()
	{
		std::vector<std::string> names;
		for (auto& [name, value] : data) names.push_back(name);
		return names;
	}
	void CVar::LoadConfigXML(const std::string& path, bool clearRegistry)
	{
		if (clearRegistry) data.clear();
		std::string xmlString = cs_std::text_file(path).open().read();
		cs_std::xml::document document(xmlString);
		for (const auto& var : document)
		{
			CVarTypeFlag type = DetectType(var->innerText);
			if (type == CVarTypeFlag::INT)
			{
				Register<int64_t>(var->tag, var->innerText);
				cs_std::console::info("Loaded CVar: int64_t ", var->tag, " = ", var->innerText); 
			}
			else if (type == CVarTypeFlag::FLOAT)
			{
				Register<double>(var->tag, var->innerText);
				cs_std::console::info("Loaded CVar: double ", var->tag, " = ", var->innerText);
			}
			else if (type == CVarTypeFlag::STRING)
			{
				Register<std::string>(var->tag, var->innerText);
				cs_std::console::info("Loaded CVar: string ", var->tag, " = ", var->innerText);
			}
			else if (type == CVarTypeFlag::BOOL)
			{
				Register<bool>(var->tag, var->innerText);
				cs_std::console::info("Loaded CVar: bool ", var->tag, " = ", var->innerText);
			}
		}
	}
	std::string CVar::SerializeConfigXML()
	{
		std::string outputString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<config>\n";
		std::vector<std::string> names = GetNames();
		for (const auto& name : names) outputString += "\t<" + name + ">" + Get<std::string>(name) + "</" + name + ">\n";
		outputString += "</config>";
		return outputString;
	}
}