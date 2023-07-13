#include "Cvar.hpp"

#include "Libraries/XML/XML.hpp"

namespace Crescendo::Engine
{
	std::unordered_map<std::string, std::string> CVar::data;
	void CVar::Register(const std::string& name, const std::string& value)
	{
		data.emplace(name, value);
	}
	template <>
	std::string CVar::Get<std::string>(const std::string& name)
	{
		return data[name];
	}
	template <>
	int64_t CVar::Get<int64_t>(const std::string& name)
	{
		return std::stoll(data[name]);
	}
	template <>
	double CVar::Get<double>(const std::string& name)
	{
		return std::stod(data[name]);
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
	void CVar::LoadConfigXML(const std::string& path)
	{
		Tools::XML::Document document = Tools::XML::ParseFromFile(path);

		for (uint32_t i = 0, childCount = document.root->GetChildCount(); i < childCount; i++)
		{
			Tools::XML::Node* child = document.root->GetChild(i);
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