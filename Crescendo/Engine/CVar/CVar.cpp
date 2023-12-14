#include "Cvar.hpp"

#include "cs_std/xml/xml.hpp"
#include "cs_std/file.hpp"

CS_NAMESPACE_BEGIN
{
	std::unordered_map<std::string, std::string> CVar::data {};
	void CVar::Register(const std::string& name, const std::string& value)
	{
		data.emplace(name, value);
	}
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
		for (const auto& var : document) Register(var->tag, var->innerText);
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