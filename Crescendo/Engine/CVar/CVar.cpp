#include "Cvar.hpp"

#include "cs_std/xml/xml.hpp"

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
		for (auto& [name, value] : data)
		{
			names.push_back(name);
		}
		return names;
	}
	void CVar::LoadConfigXML(const std::string& path, bool clearRegistry)
	{
		if (clearRegistry) { data.clear(); }

		cs_std::xml::document document = cs_std::xml::parse_file(path);

		for (uint32_t i = 0, childCount = document.root->GetChildCount(); i < childCount; i++)
		{
			cs_std::xml::node* child = document.root->GetChild(i);
			Register(child->GetTagName(), child->GetTextContent());
		}
	}
	std::string CVar::SerializeConfigXML()
	{
		std::string outputString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<config>\n";
		std::vector<std::string> names = GetNames();
		for (const auto& name : names)
		{
			outputString += "\t<" + name + ">" + Get<std::string>(name) + "</" + name + ">\n";
		}
		outputString += "</config>";
		return outputString;
	}
}